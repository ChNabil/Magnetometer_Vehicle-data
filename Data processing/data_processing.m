x1 = xlsread('Book1',2,'A1:A6502'); % reading x,y,z data for both sensors
x1 = typecast(uint16(x1),'int16');  % converting data to int16 form uint16
y1 = xlsread('Book1',2,'B1:B6502');
y1 = typecast(uint16(y1),'int16');
z1 = xlsread('Book1',2,'C1:C6502');
z1 = typecast(uint16(z1),'int16');
x2 = xlsread('Book1',2,'D1:D6502');
x2 = typecast(uint16(x2),'int16');
y2 = xlsread('Book1',2,'E1:E6502');
y2 = typecast(uint16(y2),'int16');
z2 = xlsread('Book1',2,'F1:F6502');
z2 = typecast(uint16(z2),'int16');
%distance = xlsread('Book1',2,'G1:G6502');
% subplot(321)
% plot(x1)
% subplot(323)
% plot(y1)
% subplot(325)
% plot(z1)
% subplot(322)
% plot(x2)
% subplot(324)
% plot(y2)
% subplot(326)
% plot(z2)
x1 = double(x1);  % needs to be double before squaring, or exceeds limit
y1 = double(y1);
z1 = double(z1);
mag1 = sqrt(x1.^2+y1.^2+z1.^2);
mag1 = mag1*2000/65536; % converting the value into uT
x2 = double(x2);  % needs to be double before squaring, or exceeds limit
y2 = double(y2);
z2 = double(z2);
mag2 = sqrt(x2.^2+y2.^2+z2.^2);
mag2 = mag2*2000/65536;
subplot(211)
plot(mag1)
subplot(212)
plot(mag2)