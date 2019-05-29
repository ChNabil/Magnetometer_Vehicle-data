speed = 1:100;  %actual speed (km/h)
fs = 10000;  % sampling frequency
dis_2_mag = 2;  % distance between 2 magnetometers
error_sample = -3:3;  % error in estimated time delay in terms of number of samples
%temp = dis_2_mag*fs*3.6;    % one teem of the final equation

for(k=1:4)
    switch k
        case 1
            fs = 200;
        case 2
            fs = 500;
        case 3
            fs =1000;
        case 4
            fs = 2000;
    end
    temp = dis_2_mag*fs*3.6;    % one teem of the final equation
    for(i = 1:100)
        for(j = 1:7)
            error_est_speed(j,i) = speed(i)-((temp*speed(i))/(temp+(error_sample(j)*speed(i)))); % error in estimated speed (km/h)
        end
    end
    figure()
    hold on
    xlabel('Speed')
    ylabel('Error in estimated speed')
    title(fs)
    for(i = 1:7)
        plot(error_est_speed(8-i,:))
    end
    legend('-3','-2','-1','0','1','2','3')
end