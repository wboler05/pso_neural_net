clc;
clear;

x = linspace(-1, 1, 200);
mean = .5;
sigma = 0.2;
%y = exp(-((x-mean)./sigma).^2);
%y = (4*x.^2) - (4*x) + 1;
y = exp(-((x./sigma).^2));
plot(x, y)
grid;