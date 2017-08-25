clc;
clear;

len = 200;
x = linspace(-1, 1, len);
mean = .95;
sigma = 5;
%y = exp(-((x-mean)./sigma).^2);
%y = (4*x.^2) - (4*x) + 1;


%y = 0.5*(1-exp(-((x./sigma).^2)));
%y = tanh(x*pi);
y = 1 ./ (1 + exp(-sigma.*x));

p1 = polyfit(x, y, 1);
p2 = polyfit(x, y, 2);
p3 = polyfit(x, y, 3);
p4 = polyfit(x, y, 4);
p5 = polyfit(x, y, 5);
p6 = polyfit(x, y, 6);
p7 = polyfit(x, y, 7);
p8 = polyfit(x, y, 8);
p9 = polyfit(x, y, 9);

o1 = polyval(p1, x);
o2 = polyval(p2, x);
o3 = polyval(p3, x);
o4 = polyval(p4, x);
o5 = polyval(p5, x);
o6 = polyval(p6, x);
o7 = polyval(p7, x);
o8 = polyval(p8, x);
o9 = polyval(p9, x);

%{
for i=1:len
    if x(i) < 0
        y(i) = -y(i);
    end
end
%}

subplot(2, 5, 1);
plot(x, y)
grid;

subplot(2,5,2);
plot(x, o1);
grid;

subplot(2,5,3);
plot(x, o2);
grid;

subplot(2,5,4);
plot(x, o3);
grid;

subplot(2,5,5);
plot(x, o4);
grid;

subplot(2,5,6);
plot(x, o5);
grid;

subplot(2,5,7);
plot(x, o6);
grid;

subplot(2,5,8);
plot(x, o7);
grid;

subplot(2,5,9);
plot(x, o8);
grid;

subplot(2,5,10);
plot(x, o9);
grid;
