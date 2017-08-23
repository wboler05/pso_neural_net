clc;
clear;

x = linspace(-2, 2, 200);
mean = .95;
sigma = 0.35;
%y = exp(-((x-mean)./sigma).^2);
%y = (4*x.^2) - (4*x) + 1;
y = 1-exp(-((x./sigma).^2));
plot(x, y)
grid;