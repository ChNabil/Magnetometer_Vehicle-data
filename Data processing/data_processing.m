fs = 240.6;  % sampling frequency
dis_snsrs = .11; % distance between 2 sensors in m

x1 = xlsread('Book7',2,'A:A'); % reading x,y,z data for both sensors
x1 = typecast(uint16(x1),'int16');  % converting data to int16 form uint16
y1 = xlsread('Book7',2,'B:B');
y1 = typecast(uint16(y1),'int16');
z1 = xlsread('Book7',2,'C:C');
z1 = typecast(uint16(z1),'int16');
x2 = xlsread('Book7',2,'D:D');
x2 = typecast(uint16(x2),'int16');
y2 = xlsread('Book7',2,'E:E');
y2 = typecast(uint16(y2),'int16');
z2 = xlsread('Book7',2,'F:F');
z2 = typecast(uint16(z2),'int16');
distance = xlsread('Book7',2,'G:G'); % in cm
x1 = double(x1);  % needs to be double before squaring, or exceeds limit
y1 = double(y1);
z1 = double(z1);
mag1 = sqrt(x1.^2+y1.^2+z1.^2);
mag1 = medfilt1(mag1,5);    %   median filter to remove random spikes
%mag1 = mag1*2000/65536; % converting the value into uT
x2 = double(x2);  % needs to be double before squaring, or exceeds limit
y2 = double(y2);
z2 = double(z2);
mag2 = sqrt(x2.^2+y2.^2+z2.^2);
mag2 = medfilt1(mag2,5);    %   median filter to remove random spikes
%mag2 = mag2*2000/65536;
t=0:1/fs:(size(mag1)/fs)-1/fs; % converting sample number to sec, for plotting
figure(1)
subplot(311)
plot(t,mag1)
%xlabel('Time (s)')
ylabel('Mag. field (uT)')
legend('Magnetometer 1')
subplot(312)
plot(t,mag2)
%xlabel('Time (s)')
ylabel('Mag. field (uT)')
legend('Magnetometer 2')
subplot(313)
plot(t,distance)
xlabel('Time (s)')
ylabel('Distance (cm)')
legend('Ultrasonic sensor')

%calculate speed and length of the vehicle
[cor, lag] = xcorr((mag1-mean(mag1)),(mag2-mean(mag2)));
[~,i] = max(abs(cor));
lag_time = abs(lag(i))/fs;  % lag in samples/sampling fr. lagtime in sec
speed = dis_snsrs/lag_time; % speed of vehicle in m/s
