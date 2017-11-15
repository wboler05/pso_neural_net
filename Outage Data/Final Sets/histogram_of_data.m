clc; clear;
pathToData = 'C:\Users\wboler\pso_neural_net\Outage Data\Final Sets'
addpath(pathToData);
dataIn = csvread('ECE570_Final_Dataset.csv',2);
affectedCustomers = dataIn(:, 30);

bins = 5;
maxAff = max(affectedCustomers);
minAff = min(affectedCustomers);

maxLog = log(maxAff+1);
minLog = log(minAff+1);
logRange = maxLog - minLog;
logSteps = logRange / bins;

histList = zeros(bins, 1);

for i=1:bins
    minCutoff = 10^((i-1)*logSteps) - 1;
    maxCutoff = 10^(i*logSteps);
    for j=1:length(affectedCustomers)
        if (affectedCustomers(j) >= minCutoff && affectedCustomers(j) < maxCutoff)
            histList(i) = histList(i) + 1;
        end
    end
end

x = 10.^(logSteps.*(1:5));

bar(x, histList);
set(gca, 'YScale', 'log')
%set(gca, 'XScale', 'log')
grid;
            
