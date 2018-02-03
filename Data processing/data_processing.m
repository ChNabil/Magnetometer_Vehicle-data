x1 = xlsread('Book1',2,'A1:A2581'); % reading x,y,z data for both sensors
x1 = typecast(uint16(x1),'int16');  % converting data to int16 form uint16
y1 = xlsread('Book1',2,'B1:B2581');
y1 = typecast(uint16(y1),'int16');
z1 = xlsread('Book1',2,'C1:C2581');
z1 = typecast(uint16(z1),'int16');
x2 = xlsread('Book1',2,'D1:D2581');
x2 = typecast(uint16(x2),'int16');
y2 = xlsread('Book1',2,'E1:E2581');
y2 = typecast(uint16(y2),'int16');
z2 = xlsread('Book1',2,'F1:F2581');
z2 = typecast(uint16(z2),'int16');
fs = 61.5;  % sampling frequency
dis_snsrs = .055; % distance between 2 sensors in m
distance = xlsread('Book1',2,'G1:G2581');
x1 = double(x1);  % needs to be double before squaring, or exceeds limit
y1 = double(y1);
z1 = double(z1);
mag1 = sqrt(x1.^2+y1.^2);
mag1 = mag1*2000/65536; % converting the value into uT
x2 = double(x2);  % needs to be double before squaring, or exceeds limit
y2 = double(y2);
z2 = double(z2);
mag2 = sqrt(x2.^2+y2.^2);
mag2 = mag2*2000/65536;
%mag2 = -mag2;   % because 2 sensors are positioned in opposite way
%not needed anymore, changed the position of the second sensor
subplot(311)
plot(mag1)
subplot(312)
plot(mag2)
subplot(313)
plot(distance)

%calculate speed and length of the vehicle
[cor, lag] = xcorr((mag1-mean(mag1)),(mag2-mean(mag2)));
[~,i] = max(abs(cor));
lag_time = abs(lag(i))/fs;  % lag in samples/sampling fr. lagtime in sec
speed = dis_snsrs/lag_time; % speed of vehicle in m/s
