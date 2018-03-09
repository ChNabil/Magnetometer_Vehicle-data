fs = 240.6;  % sampling frequency
dis_snsrs = .26; % distance between 2 sensors in m

x1 = xlsread('Book2',3,'A:A'); % reading x,y,z data for both sensors
x1 = typecast(uint16(x1),'int16');  % converting data to int16 form uint16
y1 = xlsread('Book2',3,'B:B');
y1 = typecast(uint16(y1),'int16');
z1 = xlsread('Book2',3,'C:C');
z1 = typecast(uint16(z1),'int16');
x2 = xlsread('Book2',3,'D:D');
x2 = typecast(uint16(x2),'int16');
y2 = xlsread('Book2',3,'E:E');
y2 = typecast(uint16(y2),'int16');
z2 = xlsread('Book2',3,'F:F');
z2 = typecast(uint16(z2),'int16');
distance = xlsread('Book2',3,'G:G'); % in cm

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
ylabel('Mag. field')
subplot(323)
plot(t,y1)
ylabel('Mag. field')
subplot(325)
plot(t,z1)
ylabel('Mag. field')
subplot(322)
plot(t,x2)
ylabel('Mag. field')
subplot(324)
plot(t,y2)
ylabel('Mag. field')
subplot(326)
plot(t,z2)
ylabel('Mag. field')
xlabel('Time (s)')

% make the signals 0 mean, otherwise moving avg filter and
% cross-correlation wont work
x1_filt = x1-mean(x1);
y1_filt = y1-mean(y1);
z1_filt = z1-mean(z1);
x2_filt = x2-mean(x2);
y2_filt = y2-mean(y2);
z2_filt = z2-mean(z2);

% moving average filter
x1_filt = conv(x1_filt, ones(101,1)/101, 'same');
y1_filt = conv(y1_filt, ones(101,1)/101, 'same');
z1_filt = conv(z1_filt, ones(101,1)/101, 'same');
x2_filt = conv(x2_filt, ones(101,1)/101, 'same');
y2_filt = conv(y2_filt, ones(101,1)/101, 'same');
z2_filt = conv(z2_filt, ones(101,1)/101, 'same');

% plot filtered data
figure()
subplot(321)
plot(t,x1_filt)
ylabel('Mag. field')
subplot(323)
plot(t,y1_filt)
ylabel('Mag. field')
subplot(325)
plot(t,z1_filt)
ylabel('Mag. field')
subplot(322)
plot(t,x2_filt)
ylabel('Mag. field')
subplot(324)
plot(t,y2_filt)
ylabel('Mag. field')
subplot(326)
plot(t,z2_filt)
ylabel('Mag. field')
xlabel('Time (s)')

mag1 = sqrt(x1.^2+y1.^2+z1.^2);
mag1 = mag1-mean(mag1);
mag1 = conv(mag1, ones(101,1)/101, 'same'); % moving avg filter
% mag1 = medfilt1(mag1,5);    %   median filter to remove random spikes
%mag1 = mag1*2000/65536; % converting the value into uT
mag2 = sqrt(x2.^2+y2.^2+z2.^2);
mag2 = mag2-mean(mag2);
mag2 = conv(mag2, ones(101,1)/101, 'same'); % moving avg filter
% mag2 = medfilt1(mag2,5);    %   median filter to remove random spikes
%mag2 = mag2*2000/65536;
figure()
subplot(311)
plot(t,mag1)
ylabel('Mag. field')
legend('Magnetometer 1')
subplot(312)
plot(t,mag2)
ylabel('Mag. field')
legend('Magnetometer 2')
subplot(313)
plot(t,distance)
%xlabel('Time (s)')
xlabel('Time (s)')
ylabel('Distance (cm)')
legend('Ultrasonic sensor')

%calculate speed and length of the vehicle
[cor, lag] = xcorr(mag1,mag2);
[~,i] = max(abs(cor));
lag_time = abs(lag(i))/fs;  % lag in samples/sampling fr. lagtime in sec
speed = dis_snsrs/lag_time*3.6; % speed of vehicle in km/h
