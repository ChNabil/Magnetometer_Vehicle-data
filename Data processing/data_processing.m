fs = 200;  % sampling frequency
dis_snsrs = .5; % distance between 2 sensors in m

x1 = xlsread('capture0_32kmph',3,'A:A'); % reading x,y,z data for both sensors
x1 = typecast(uint16(x1),'int16');  % converting data to int16 form uint16
y1 = xlsread('capture0_32kmph',3,'B:B');
y1 = typecast(uint16(y1),'int16');
z1 = xlsread('capture0_32kmph',3,'C:C');
z1 = typecast(uint16(z1),'int16');
x2 = xlsread('capture0_32kmph',3,'D:D');
x2 = typecast(uint16(x2),'int16');
y2 = xlsread('capture0_32kmph',3,'E:E');
y2 = typecast(uint16(y2),'int16');
z2 = xlsread('capture0_32kmph',3,'F:F');
z2 = typecast(uint16(z2),'int16');
%distance = xlsread('capture0_32kmph',3,'G:G'); % in cm

% converting sample number to sec, for plotting
%t = 0:1/fs:(size(x1)/fs)-1/fs; 
t = [0:size(x1)-1]/fs;

% needs to be double before squaring, or exceeds limit when squaring
x1 = double(x1);  
y1 = double(y1);
z1 = double(z1);
x2 = double(x2);
y2 = double(y2);
z2 = double(z2);

figure()
subplot(321)
plot(t,x1)
ylabel('Mag. field_x (µT)')
legend('Magnetometer 1')
subplot(323)
plot(t,y1)
ylabel('Mag. field_y (µT)')
subplot(325)
plot(t,z1)
ylabel('Mag. field_z (µT)')
xlabel('Time (s)')
subplot(322)
plot(t,x2)
% ylabel('Mag. field(x)')
legend('Magnetometer 2')
subplot(324)
plot(t,y2)
% ylabel('Mag. field(y)')
subplot(326)
plot(t,z2)
% ylabel('Mag. field(z)')
xlabel('Time (s)')

% median filter to remove random spikes
x1 = medfilt1(x1,3);
y1 = medfilt1(y1,3);
z1 = medfilt1(z1,3);
x2 = medfilt1(x2,3);
y2 = medfilt1(y2,3);
z2 = medfilt1(z2,3);

% plot raw data
figure()
subplot(321)
plot(t,x1)
ylabel('Mag. field_x (µT)')
legend('Magnetometer 1')
subplot(323)
plot(t,y1)
ylabel('Mag. field_y (µT)')
subplot(325)
plot(t,z1)
ylabel('Mag. field_z (µT)')
xlabel('Time (s)')
subplot(322)
plot(t,x2)
% ylabel('Mag. field(x)')
legend('Magnetometer 2')
subplot(324)
plot(t,y2)
% ylabel('Mag. field(y)')
subplot(326)
plot(t,z2)
% ylabel('Mag. field(z)')
xlabel('Time (s)')

% make the signals 0 mean, otherwise moving avg filter and
% cross-correlation wont work
% x1_filt = x1-mean(x1);
% y1_filt = y1-mean(y1);
% z1_filt = z1-mean(z1);
% x2_filt = x2-mean(x2);
% y2_filt = y2-mean(y2);
% z2_filt = z2-mean(z2);

mag1 = sqrt(x1.^2+y1.^2+z1.^2);
mag2 = sqrt(x2.^2+y2.^2+z2.^2);
% mag1 = mag1-mean(mag1);
% mag2 = mag2-mean(mag2);
% mag1 = conv(mag1, ones(25,1)/25, 'same'); % moving avg filter
% mag2 = conv(mag2, ones(25,1)/25, 'same'); % moving avg filter

% moving average filter
% x1_filt = conv(x1_filt, ones(25,1)/25, 'same');
% y1_filt = conv(y1_filt, ones(25,1)/25, 'same');
% z1_filt = conv(z1_filt, ones(25,1)/25, 'same');
% x2_filt = conv(x2_filt, ones(25,1)/25, 'same');
% y2_filt = conv(y2_filt, ones(25,1)/25, 'same');
% z2_filt = conv(z2_filt, ones(25,1)/25, 'same');

% % plot filtered data
% figure()
% subplot(321)
% plot(t,x1_filt)
% ylabel('Mag. field(x)')
% subplot(323)
% plot(t,y1_filt)
% ylabel('Mag. field(y)')
% subplot(325)
% plot(t,z1_filt)
% ylabel('Mag. field(z)')
% xlabel('Time (s)')
% subplot(322)
% plot(t,x2_filt)
% %ylabel('Mag. field(x)')
% subplot(324)
% plot(t,y2_filt)
% %ylabel('Mag. field')
% subplot(326)
% plot(t,z2_filt)
% %ylabel('Mag. field')
% xlabel('Time (s)')

% mag1 = sqrt(x1_filt.^2+y1_filt.^2+z1_filt.^2);
% mag1 = sqrt(x1.^2+y1.^2+z1.^2);
% mag1 = mag1-mean(mag1);
% mag1 = conv(mag1, ones(5,1)/5, 'same'); % moving avg filter
% mag1 = medfilt1(mag1,5);    %   median filter to remove random spikes
%mag1 = mag1*2000/65536; % converting the value into uT
% mag2 = sqrt(x2_filt.^2+y2_filt.^2+z2_filt.^2);
% mag2 = sqrt(x2.^2+y2.^2+z2.^2);
% mag2 = mag2-mean(mag2);
% mag2 = conv(mag2, ones(5,1)/5, 'same'); % moving avg filter
% mag2 = medfilt1(mag2,5);    %   median filter to remove random spikes
%mag2 = mag2*2000/65536;
figure()
subplot(211)
plot(t,mag1)
ylabel('Mag. field (µT)')
legend('Magnetometer 1')
subplot(212)
plot(t,mag2)
ylabel('Mag. field (µT)')
legend('Magnetometer 2')
%subplot(313)
%plot(t,distance)
%xlabel('Time (s)')
xlabel('Time (s)')
%ylabel('Distance (cm)')
%legend('Ultrasonic sensor')
%ylabel('Vibration (m/s^2)')
%legend('Accelerometer')

mag1_th=(sum(mag1(1:100)))/100;
mag2_th=(sum(mag2(1:100)))/100;

mag1 = mag1 - mag1_th;
mag2 = mag2 - mag2_th;

%calculate speed and length of the vehicle
[cor, lag] = xcorr(mag1,mag2);
[~,i] = max(abs(cor));
lag_time = abs(lag(i))/fs;  % lag in samples/sampling fr. lagtime in sec
% [x a]=max(mag1);
% [x b]=max(mag2);
% lag_time = abs(b-a)/fs;

qq=0;
qq1=0;
ll=length(mag1);
for(ttt=1:ll-10)
    for(tttt=0:9)
         if (mag1(ttt+tttt)>3)
           qq=qq+1;
         end
         if mag2(ttt+tttt)>3
           qq1=qq1+1;
         end
         if(qq==10)
             zz1=ttt;
         end
         if(qq1==10)
             zz2=ttt;
         end
    end
end
    
lag_time1=(zz2-zz1)/fs;
speed = dis_snsrs/lag_time*3.6; % speed of vehicle in km/h
speed1 = dis_snsrs/lag_time1*3.6; % speed of vehicle in km/h

l=0;
for(tt=1:length(mag1))
    if mag1(tt)>4 || mag1(tt)<-4
        l=l+1;
    end
end

len = l/200*speed/3.6; % length of the vehicel in m
len1 = l/200*speed1/3.6; % length of the vehicel in m

figure()
subplot(211)
plot(t,mag1)
ylabel('Mag. field')
legend('Magnetometer 1')
subplot(212)
plot(t,mag2)
ylabel('Mag. field')
legend('Magnetometer 2')
%subplot(313)
%plot(t,distance)
%xlabel('Time (s)')
xlabel('Time (s)')