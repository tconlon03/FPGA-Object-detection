function background_golden_reference_gen()
 
    videos = {'C:\\Users\\Tiarnan\\Pictures\\Matlab_test\\test_vid_1.avi',
        'C:\\Users\\Tiarnan\\Pictures\\Matlab_test\\test_vid_2.avi',
        'C:\\Users\\Tiarnan\\Pictures\\Matlab_test\\test_vid_3.avi',
        'C:\\Users\\Tiarnan\\Pictures\\Matlab_test\\test_vid_4.avi',
        'C:\\Users\\Tiarnan\\Pictures\\Matlab_test\\test_vid_5.avi'}
    start = [500,1,715,1,1];
    for i = 1:5
        obj = setUpSystemObjects(char(videos(i)));

        count = 0;
        bad = 0;
        good = 0;
        base_path = 'C:\\Users\\Tiarnan\\Pictures\\bg_core_output\\test_vid_';
        %fid = fopen('GoldenReference.txt','wt');
        %p_c = 0;
        while ~isDone(obj.reader)
            count = count + 1;
            frame = obj.reader();
            if (count < start(i))
                continue;
            end
            mask = detectMovement(frame);  
            path = sprintf('%s%d\\%d_%s', base_path, i, count, 'mask.png');
            hls_mask = imread(path);
            height = size(frame,1);
            width = size(frame, 2);
            obj.maskPlayer.step(mask);
            obj.videoPlayer.step(frame);
            fg_frame = 0;
            good_frame = 0;
            bad_frame = 0;
                for row = 1:height
                    for col = 1:width
                        if (mask(row,col) > 0)
                            fg_frame = fg_frame + 1;
                            if hls_mask(row,col) > 0
                                good_frame = good_frame + 1;
                            else 
                                bad_frame = bad_frame + 1;
                            end
                        else
                            if hls_mask(row,col) == 0
                                good_frame = good_frame + 1;
                            else 
                                bad_frame = bad_frame + 1;
                            end
                        end
                    end
                end
                if ((fg_frame > height*width*0.03) & (fg_frame < height*width*0.5))
                    good = good + good_frame;
                    bad = bad + bad_frame;
                end
        end
        good 
        bad
        ratio = 100 - (bad / (good+bad) * 100) 
    end
    
    function obj = setUpSystemObjects(videopath)   
        obj.reader = vision.VideoFileReader(videopath);
        obj.maskPlayer = vision.VideoPlayer('Position', [740, 400, 700, 400]);
        obj.videoPlayer = vision.VideoPlayer('Position', [20, 400, 700, 400]);
        obj.detector = vision.ForegroundDetector('NumGaussians',3,'NumTrainingFrames', 50, 'MinimumBackgroundRatio', 0.6, 'InitialVariance', 1, 'LearningRate', 0.005);
    end
 
    function mask = detectMovement(frame)
        mask = obj.detector(frame);
    end
end