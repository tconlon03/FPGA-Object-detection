function back_sub_kalman()

    obj = setUpSystemObjects();

    tracks = struct(...
        'id', {}, ...
        'bbox', {}, ...
        'contour', {}, ...
        'kalmanFilter', {}, ...
        'age', {}, ...
        'totalVisibleCount', {}, ...
        'invisibleCount', {}, ...
        'track', {} ...
    );

    nextId = 1;
    
    radius = 200;
    open(obj.k);
    
    count = 0;
    overlap = -1;
    
    while ~isDone(obj.reader)
        frame = obj.reader();    
        count = count + 1;
        if (count < 500)
             continue;
        end
        mask = detectMovement(frame);
        [~, centroids, bboxes] = obj.blobAnalyser(mask);
        mask_p=bwperim(mask);
        predictNewLocationsOfTracks();
        [assignments, unassignedTracks, unassignedDetections] = detectionToTrackAssignment();
        updateAssignedTracks();
        updateUnassignedTracks();
        deleteLostTracks();
        if ~isempty(centroids) 
            createNewTracks();
        end
        motion = predictMotion();
        checkForOverlap(motion);  
        displayTrackingResults(mask_p);
    end
    
    close(obj.k)

    function obj = setUpSystemObjects()    
        obj.reader = vision.VideoFileReader('C:\Users\Tiarnan\Pictures\Matlab_test\test_vid_5.mp4');
        obj.maskPlayer = vision.VideoPlayer('Position', [740, 400, 700, 400]);
        obj.videoPlayer = vision.VideoPlayer('Position', [20, 400, 700, 400]);
        obj.detector = vision.ForegroundDetector('NumGaussians', 7,'NumTrainingFrames', 50, 'MinimumBackgroundRatio', 0.6, 'InitialVariance', 1);
        obj.blobAnalyser = vision.BlobAnalysis('BoundingBoxOutputPort', true, 'AreaOutputPort', true, 'CentroidOutputPort', true, ...
            'MinimumBlobArea',7000);
        obj.k = VideoWriter('C:\Users\Tiarnan\Pictures\Matlab_test\output_test_vid_3.mp4'); 
    end

    function mask = detectMovement(frame)
        mask = obj.detector(frame);
        mask = imopen(mask, strel('rectangle', [3,3]));
        mask = imclose(mask, strel('rectangle', [8, 8]));
        mask = bwareaopen(mask, 50);
        mask = imfill(mask, 'holes');
    end

    function predictNewLocationsOfTracks()
        for i = 1:length(tracks)
            bbox = tracks(i).bbox;

            % Predict the current location of the track.
            predictedCentroid = predict(tracks(i).kalmanFilter);

            %push the predicted centroid onto the tracks track
            tracks(i).track(end + 1) = predictedCentroid(1);
            tracks(i).track(end + 1) = predictedCentroid(2);
            
            % Shift the bounding box so that its center is at
            % the predicted location.
            predictedCentroid = int32(predictedCentroid) - bbox(3:4) / 2;
            tracks(i).bbox = [predictedCentroid, bbox(3:4)];
        end
    end

   function [assignments, unassignedTracks, unassignedDetections] = detectionToTrackAssignment()
        nTracks = length(tracks);
        nDetections = size(centroids, 1);

        % Compute the cost of assigning each detection to each track.
        cost = zeros(nTracks, nDetections);
        for i = 1:nTracks
            cost(i, :) = distance(tracks(i).kalmanFilter, centroids);
        end

        % Solve the assignment problem.
        costOfNonAssignment = 20;
        [assignments, unassignedTracks, unassignedDetections] = assignDetectionsToTracks(cost, costOfNonAssignment);
   end

    function updateAssignedTracks()
            numAssignedTracks = size(assignments, 1);
            for i = 1:numAssignedTracks
                trackIdx = assignments(i, 1);
                if (tracks(trackIdx).id < 0 & nextId == 12)
                    continue;
                end
                detectionIdx = assignments(i, 2);
                centroid = centroids(detectionIdx, :);
                bbox = bboxes(detectionIdx, :);

                % Correct the estimate of the object's location
                % using the new detection.
                correct(tracks(trackIdx).kalmanFilter, centroid);
                
                %push the real centroid onto the tracks track
                tracks(trackIdx).track(end-1) = centroid(1);
                tracks(trackIdx).track(end) = centroid(2);

                % Replace predicted bounding box with detected
                % bounding box.
                tracks(trackIdx).bbox = bbox;
%                 tracks(trackIdx).contour = contour;
                
                % Update track's frame count.
                tracks(trackIdx).age = tracks(trackIdx).age + 1;

                % Update visibility.
                tracks(trackIdx).totalVisibleCount = ...
                    tracks(trackIdx).totalVisibleCount + 1;
                tracks(trackIdx).invisibleCount = 0;
                if (tracks(trackIdx).totalVisibleCount > 15) && (tracks(trackIdx).id < 0)
                    tracks(trackIdx).id = nextId;
                    % Increment the next id.
                    nextId = nextId + 1;        
                    %get contour here from getContours
                end
            end
    end

     function updateUnassignedTracks()
            for i = 1:length(unassignedTracks)
                ind = unassignedTracks(i);
                tracks(ind).age = tracks(ind).age + 1;
                tracks(ind).invisibleCount = ...
                    tracks(ind).invisibleCount + 1;
            end
     end

    function deleteLostTracks()
        if isempty(tracks)
            return;
        end

        invisibleForTooLong = 50;
        ageThreshold = 20;

        % Compute the fraction of the track's age for which it was visible.
        ages = [tracks(:).age];
        totalVisibleCounts = [tracks(:).totalVisibleCount];
        visibility = totalVisibleCounts ./ ages;

        % Find the indices of 'lost' tracks.
        lostInds = (ages < ageThreshold & visibility < 0.5) | [tracks(:).invisibleCount] >= invisibleForTooLong;

        % Delete lost tracks.
        tracks = tracks(~lostInds);
    end

    function createNewTracks()
        centroids = centroids(unassignedDetections, :);
        bboxes = bboxes(unassignedDetections, :);
    %         contours = contours(unassignedDetections, :);

        for i = 1:size(centroids, 1)

            centroid = centroids(i,:);
            bbox = bboxes(i, :);
    %             contour = contours(i,:);

            % Create a Kalman filter object.
%             kalmanFilter = configureKalmanFilter('ConstantVelocity', ...
%                 centroid, [200, 50], [100, 25], 100);
              kalmanFilter = configureKalmanFilter('ConstantAcceleration',...
                  centroid, [1 1 1]*1e5, [25, 10, 10], 25);
            % Create a new track id assigned after found in 20 frames.
            newTrack = struct(...
                'id', -1 , ...
                'bbox', bbox, ...
                'contour', 1, ...
                'kalmanFilter', kalmanFilter, ...
                'age', 1, ...
                'totalVisibleCount', 1, ...
                'invisibleCount', 0, ...
                'track', {centroid});
            % Add it to the array of temp_tracks.
            tracks(end + 1) = newTrack;
        end

    end

    function motion = predictMotion()
        motion = zeros(1, length(tracks));
        for i = 1:length(tracks)
            if tracks(i).id < 0 
                continue
            end
            horiz_comp = zeros(1,10);
            vert_comp = zeros(1,10);
            %get oldest to newest tracks
            counter=1;
            for j = 20:-2:1 
                if (j+1 < length(tracks(i).track))
                    xprev = tracks(i).track(end-j-1:end-j-1);
                    yprev = tracks(i).track(end-j:end-j);
                    x = tracks(i).track(end-j+1:end-j+1);
                    y = tracks(i).track(end-j+2:end-j+2);
                    horiz_comp(counter) =  (41-j)*(x-xprev);
                    vert_comp(counter) = (41-j)*(y-yprev);
                    counter = counter + 1;
                end               
            end
            total_horiz=sum(horiz_comp(1:1:end))/400;
            total_vert=sum(vert_comp(1:1:end))/400;
            X = ['track ',num2str(tracks(i).id),' horiz : ', num2str(total_horiz)];
            disp(X) 
            Y = ['track ',num2str(tracks(i).id),' vert : ', num2str(total_vert)];
            disp(Y) 
            motion(i,1) = total_vert;
            motion(i,2) = total_horiz;
        end
    end


    function checkForOverlap(motion)
        %check future
        colour = 'green';
        for i = 1:length(tracks)
            if tracks(i).id < 0  
                continue
            end
            total_vert = motion(i,1);
            total_horiz = motion(i , 2);
            slope = total_vert/total_horiz;
            c = (tracks(i).track(end:end)) - slope*(tracks(i).track(end-1:end-1));
            mag = sqrt((total_horiz*total_horiz) + (total_vert*total_vert))
            future = mag * 10
            [xout, yout] = linecirc(slope,c , 250, 930, radius);
            endx = (tracks(i).track(end-1:end-1)) + (future * total_horiz)
            endy = (tracks(i).track(end:end)) + (future * total_vert)
            dist_end = sqrt(((250-endx)*(250-endx))+((900-endy)*(900-endy)));
            dist_cp = sqrt((250-(tracks(i).track(end-1:end-1)))*(250-(tracks(i).track(end-1:end-1)))+((900-(tracks(i).track(end:end)))*(900-(tracks(i).track(end:end)))));
            if (~isnan(xout) & dist_end < dist_cp)
                disp('predicted motion in circle')
                colour = 'red';
                radius=max(200,40*mag);
            else
                radius=max(200, radius*0.8);
            end
            frame = insertShape(frame, 'line', [(tracks(i).track(end-1:end-1)), (tracks(i).track(end:end)), endx, endy], 'Color', colour, 'LineWidth',10);  
        end  
        
        %check present
        overlap = -1;
        for i = 1:length(tracks)
            if tracks(i).id < 0 
                continue
            end
            tlx = tracks(i).bbox(1:1);
            tly = tracks(i).bbox(2:2);
            w = tracks(i).bbox(3:3);
            h = tracks(i).bbox(4:4);
            tlx = double(tlx);
            tly = double(tly);
            w = double(w);
            h = double(h);
            %left_line, bottom line right line, top line
            left = [tlx, tly, tlx, tly+h];
            left = double(left);
            bottom = [tlx, tly+h, tlx+w, tly+h];
            bottom = double(left);
            right = [tlx+w, tly, tlx+w, tly+h];
            right = double(left);
            top = [tlx, tly, tlx+w, tly];
            top = double(left);      
            C = [250, 970];
            %check corner distance
            tl_dx = C(1) - tlx;
            tl_dy = C(2) -tly;
            dist = sqrt((tl_dx*tl_dx)+(tl_dy*tl_dy));
            if (dist < radius)
                overlap=1;
                return;
            end
            tr_dx = C(1) - (tlx + w);
            tr_dy = C(2) - tly;
            dist = sqrt((tr_dx*tr_dx)+(tr_dy*tr_dy));
            if (dist < radius)
                overlap=1;
                return;
            end
            bl_dx = C(1) - tlx;
            bl_dy = C(2) - (tly+h);
            dist = sqrt((bl_dx*bl_dx)+(bl_dy*bl_dy));
            if (dist < radius)
                overlap=1;
                return;
            end
            br_dx = C(1) - (tlx+w);
            br_dy = C(2) - (tly+h);
            dist = sqrt((br_dx*br_dx)+(br_dy*br_dy));
            if (dist < radius)
                overlap=1;
                return;
            end
            U_l = ((C(1) - left(1))*(left(3)-left(1))+(C(2)-left(2))*(left(4)-left(2)))/(h*h + w*w);
            U_b = ((C(1) - bottom(1))*(bottom(3)-bottom(1))+(C(2)-bottom(2))*(bottom(4)-bottom(2)))/(h*h + w*w);
            U_r = ((C(1) - right(1))*(right(3)-right(1))+(C(2)-right(2))*(right(4)-right(2)))/(h*h + w*w);
            U_t = ((C(1) - top(1))*(top(3)-top(1))+(C(2)-top(2))*(top(4)-top(2)))/(h*h + w*w);
            if ((U_l>0 && U_l<1))
                x_closest = left(1) + U_l*(left(3)-left(1)); 
                y_closest = left(2) + U_l*(left(4)-left(2));
                dx = C(1) - x_closest;
                dy = C(2) - y_closest;
                dist = sqrt((dx*dx)+(dy*dy));
                if (dist < radius)
                    overlap=1;
                    return;
                end
            end    
            if ((U_b>0 && U_b<1))
                x_closest = bottom(1) + U_b*(bottom(3)-bottom(1)); 
                y_closest = bottom(2) + U_b*(bottom(4)-bottom(2));
                dx = C(1) - x_closest;
                dy = C(2) - y_closest;
                dist = sqrt((dx*dx)+(dy*dy));
                if (dist < radius)
                    overlap=1;
                    return;
                end
            end    
            if ((U_r>0 && U_r<1))
                x_closest = right(1) + U_r*(right(3)-right(1)); 
                y_closest = right(2) + U_r*(right(4)-right(2));
                dx = C(1) - x_closest;
                dy = C(2) - y_closest;
                dist = sqrt((dx*dx)+(dy*dy));
                if (dist < radius)
                    overlap=1;
                    return;
                end
            end    
            if ((U_t>0 && U_t<1))
                x_closest = top(1) + U_t*(top(3)-top(1)); 
                y_closest = top(2) + U_t*(top(4)-top(2));
                dx = C(1) - x_closest;
                dy = C(2) - y_closest;
                dist = sqrt((dx*dx)+(dy*dy));
                if (dist < radius)
                    overlap=1;
                    return;
                end
            end    
        end
       
    end


    function displayTrackingResults(mask_p)
            % Convert the frame and the mask to uint8 RGB.
            frame = im2uint8(frame);
            mask = uint8(repmat(mask, [1, 1, 3])) .* 255;

            minVisibleCount = 15;
            if ~isempty(tracks)

                % Noisy detections tend to result in short-lived tracks.
                % Only display tracks that have been visible for more than
                % a minimum number of frames.
                reliableTrackInds = ...
                    [tracks(:).totalVisibleCount] > minVisibleCount;
                reliableTracks = tracks(reliableTrackInds);

                % Display the objects. If an object has not been detected
                % in this frame, display its predicted bounding box.
                if ~isempty(reliableTracks)
                    % Get bounding boxes.
                    bboxes = cat(1, reliableTracks.bbox);
    %                 contours = cat(1, reliableTracks.contour);
                    % Get ids.
                    ids = int32([reliableTracks(:).id]);

                    % Create labels for objects indicating the ones for
                    % which we display the predicted rather than the actual
                    % location.
                    labels = cellstr(int2str(ids'));
                    predictedTrackInds = ...
                        [reliableTracks(:).invisibleCount] > 0;
                    isPredicted = cell(size(labels));
                    isPredicted(predictedTrackInds) = {' predicted'};
                    labels = strcat(labels, isPredicted);

                    % Draw the objects on the frame.
                    objectcolor = 'yellow'
                    if overlap >= 0
                        objectcolor = 'red'
                    end
                    frame = insertObjectAnnotation(frame, 'rectangle', ...
                        bboxes, labels, 'FontSize', 20, 'LineWidth', 10, 'Color', objectcolor);
    %                 frame = shapeInserter(frame,contours);

                    % Draw the objects on the mask.
                    mask = insertObjectAnnotation(mask, 'rectangle', ...
                        bboxes, labels,'FontSize', 20, 'LineWidth', 10);
                end
            end
            frame = insertShape(frame, 'circle', [250, 970, radius], 'Color', 'red', 'LineWidth',10);  
            frame = insertShape(frame, 'line', [270, 950, 300, 1055], 'Color', 'red', 'LineWidth',10);  

            % Display the mask and the frame.
            obj.maskPlayer.step(mask);
            obj.videoPlayer.step(frame);
            writeVideo(obj.k, frame);
    end

end
