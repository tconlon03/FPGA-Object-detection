function back_sub_kalman()

    obj = setUpSystemObjects();    
    count = 0;
    step = 0;
    %read HLS detected bboxes
    fid = fopen('C:\\Users\\Tiarnan\\Documents\\Final Year Project\\test_vid_1_id_var1\\bboxes.txt','r');
    A = fscanf(fid,'%d');
    A_idx = 1;
    %skipped different numbers of frames for longer videos
    frames_skipped = 5; %test video 2
    %frames_skipped = 7;  %test video 3
    overlapRatio = 0;
    matlab_bbox_count = 0;
    hls_bbox_count = 0;
    match_count = 0;
    while ~isDone(obj.reader)
        pause_pls = 0;
        frame = obj.reader();    
        count = count + 1
        mask = detectMovement(frame);
          if (count < 885)  %test video 3
              continue;
          end
          if (count > 1395)
              break;
          end
%       from 715 we only want every 7th frame.
        if step > 0
           step = step - 1;
           continue;
        end
        %Get MATLAB bbox
        [~, centroids, bboxes] = obj.blobAnalyser(mask);
        matlab_bbox_count = matlab_bbox_count + size(centroids, 1);
        %get HLS bbox for this frame
        if A(A_idx) ~= count + 15
            %something wrong
            count
            A_idx
            break;
        else
            A_idx = A_idx + 1;
            if (A_idx > size(A))
                break;
            end
            HLS_bboxes = [];
            while A(A_idx) ~= count + frames_skipped + 15
                hls_bbox_count = hls_bbox_count + 1;
                %basically for each HLS BBOX in this frame
                %MATLAB bbox format is [upper left x, upper left y, w, h]
                %HLS bbox format is [upper left x, upper left y, bottom right x, bottom right y]
                %test vids 2&4 are 320 wide, vids 1,3,5 are 426 wide
                 HLS_bbox = [4.5*(A(A_idx)), 4.5*(A(A_idx+1)),(4.5*(A(A_idx+2))-4.5*(A(A_idx))),(4.5*(A(A_idx+3))-4.5*(A(A_idx+1)))];
%                HLS_bbox = [6*(A(A_idx)), 4.5*(A(A_idx+1)),(6*(A(A_idx+2))-6*(A(A_idx))),(4.5*(A(A_idx+3))-4.5*(A(A_idx+1)))];
                HLS_bboxes = [HLS_bboxes;HLS_bbox];
                labels = cellstr('HLS');
                HLS_bbox
                frame = insertObjectAnnotation(frame, 'rectangle', HLS_bbox, labels,'FontSize', 20, 'LineWidth',10);   
                A_idx = A_idx + 4;
                if A_idx > size(A)
                    break;
                end
            end
            if (A_idx > size(A))
                break;
            end
            for i = 1:size(centroids, 1)
                matlab_max_ratio = 0;
                for j = 1:size(HLS_bboxes,1)
                    %each bbox only matches to one bbox, the max
                    MATLAB_bbox = bboxes(i, :);
                    HLS_bbox = HLS_bboxes(j, :);
                    temp_overlapRatio = bboxOverlapRatio(HLS_bbox, MATLAB_bbox);
                    if temp_overlapRatio > matlab_max_ratio
                        matlab_max_ratio = temp_overlapRatio;
                    end
                end
                if matlab_max_ratio > 0
                    if matlab_max_ratio > 0.5 && matlab_max_ratio < 0.6
                        pause_pls = 1;
                    end
                    match_count = match_count + 1;
                    matlab_max_ratio
                end
                overlapRatio = overlapRatio + matlab_max_ratio;
            end
        end
        labels = cellstr('MATLAB');
        frame = insertObjectAnnotation(frame, 'rectangle', bboxes, labels, 'Color', 'red', 'FontSize', 20, 'LineWidth',10);       
        obj.videoPlayer.step(frame);
        pause(0.2);
        if pause_pls == 1
            pause(10)
        end
        step = frames_skipped - 1;
    end
    max(hls_bbox_count,matlab_bbox_count)
    overlapRatio 
    overlapRatio / max(hls_bbox_count,matlab_bbox_count)
    overlapRatio / match_count
    
    function obj = setUpSystemObjects()    
        obj.reader = vision.VideoFileReader('C:\Users\Tiarnan\Pictures\Matlab_test\test_vid_3.avi');
        obj.videoPlayer = vision.VideoPlayer('Position', [20, 400, 700, 400]);
        obj.detector = vision.ForegroundDetector('NumGaussians', 3,'NumTrainingFrames', 50, 'MinimumBackgroundRatio', 0.6, 'InitialVariance', 1);
        obj.blobAnalyser = vision.BlobAnalysis('BoundingBoxOutputPort', true, 'AreaOutputPort', true, 'CentroidOutputPort', true, ...
            'MinimumBlobArea',700);
    end

    function mask = detectMovement(frame)
        mask = obj.detector(frame);
        mask = imopen(mask, strel('rectangle', [3,3]));
        mask = imclose(mask, strel('rectangle', [8, 8]));
        mask = bwareaopen(mask, 50);
        mask = imfill(mask, 'holes');
    end

end
