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
%t=0:1/fs:(size(mag1)/fs)-1/fs; 

% plot raw data
figure(1)
subplot(321)
plot(x1)
subplot(323)
plot(y1)
subplot(325)
plot(z1)
subplot(322)
plot(x2)
subplot(324)
plot(y2)
subplot(326)
plot(z2)

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

% make the signals 0 mean, otherwise moving avg filter and
% cross-correlation wont work
x1 = x1-mean(x1);
y1 = y1-mean(y1);
z1 = z1-mean(z1);
x2 = x2-mean(x2);
y2 = y2-mean(y2);
z2 = z2-mean(z2);

% moving average filter
x1 = conv(x1, ones(101,1)/101, 'same');
y1 = conv(y1, ones(101,1)/101, 'same');
z1 = conv(z1, ones(101,1)/101, 'same');
x2 = conv(x2, ones(101,1)/101, 'same');
y2 = conv(y2, ones(101,1)/101, 'same');
z2 = conv(z2, ones(101,1)/101, 'same');

% plot filtered data
figure(2)
subplot(321)
plot(x1)
subplot(323)
plot(y1)
subplot(325)
plot(z1)
subplot(322)
plot(x2)
subplot(324)
plot(y2)
subplot(326)
plot(z2)

mag1 = sqrt(x1.^2+y1.^2+z1.^2);
mag1 = mag1-mean(mag1);
% mag1 = medfilt1(mag1,5);    %   median filter to remove random spikes
%mag1 = mag1*2000/65536; % converting the value into uT
mag2 = sqrt(x2.^2+y2.^2+z2.^2);
mag2 = mag2-mean(mag2);
% mag2 = medfilt1(mag2,5);    %   median filter to remove random spikes
%mag2 = mag2*2000/65536;
figure(3)
subplot(311)
plot(mag1)
ylabel('Mag. field')
legend('Magnetometer 1')
subplot(312)
plot(mag2)
ylabel('Mag. field')
legend('Magnetometer 2')
subplot(313)
plot(distance)
%xlabel('Time (s)')
xlabel('Sample')
ylabel('Distance (cm)')
legend('Ultrasonic sensor')

%calculate speed and length of the vehicle
[cor, lag] = xcorr(mag1,mag2);
[~,i] = max(abs(cor));
lag_time = abs(lag(i))/fs;  % lag in samples/sampling fr. lagtime in sec
speed = dis_snsrs/lag_time; % speed of vehicle in m/s
