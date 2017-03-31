%% ----------------------------------------------------------------------------
% PSOGSA source codes version 1.0.
% Author: Seyedali Mirjalili (ali.mirjalili@gmail.com)

% Main paper:
% S. Mirjalili, S. Z. Mohd Hashim, and H. Moradian Sardroudi, "Training 
%feedforward neural networks using hybrid particle swarm optimization and 
%gravitational search algorithm," Applied Mathematics and Computation, 
%vol. 218, pp. 11125-11137, 2012.

%The paper of the PSOGSA algorithm utilized as the trainer:
%S. Mirjalili and S. Z. Mohd Hashim, "A New Hybrid PSOGSA Algorithm for 
%Function Optimization," in International Conference on Computer and Information 
%Application?ICCIA 2010), 2010, pp. 374-377.

% Adding the Clotfinder Data to this procedure for FNN training
%% -----------------------------------------------------------------------------

clc
clear all
close all

%% ////////////////////////////////////////////////////Data set preparation/////////////////////////////////////////////
 in=xlsread('clean.xlsx');
 randomorder=randperm(size(in,1));
 inputrand=in(randomorder,:);
 indata=inputrand(1:2101,2:11);
 EtCO2=inputrand(1:2101,2);
 Age=inputrand(1:2101,3);
 SystBP=inputrand(1:2101,4);
 SaO2=inputrand(1:2101,5);
 HEM=inputrand(1:2101,6);
 TOBAC=inputrand(1:2101,7);
 SURG=inputrand(1:2101,8);
 CARD=inputrand(1:2101,9);
 DVT=inputrand(1:2101,10);
 ULS=inputrand(1:2101,11);
 PE=inputrand(1:2101,1);
 
 EtCO2=EtCO2';
 [xf,PS1] = mapminmax(EtCO2);
 I2(:,1)=xf;
 
 Age=Age';
 [xf,PS2] = mapminmax(Age);
 I2(:,2)=xf;
 
 SystBP=SystBP';
 [xf,PS3] = mapminmax(SystBP);
 I2(:,3)=xf;
 
 SaO2=SaO2';
 [xf,PS4] = mapminmax(SaO2);
 I2(:,4)=xf;
 
 HEM=HEM';
 [xf,PS5] = mapminmax(HEM);
 I2(:,5)=xf;
 
 TOBAC=TOBAC';
 [xf,PS6] = mapminmax(TOBAC);
 I2(:,6)=xf;
 
 SURG=SURG';
 [xf,PS7] = mapminmax(SURG);
 I2(:,7)=xf;
 
 CARD=CARD';
 [xf,PS8] = mapminmax(CARD);
 I2(:,8)=xf;
 
 DVT=DVT';
 [xf,PS9] = mapminmax(DVT);
 I2(:,9)=xf;
 
 ULS=ULS';
 [xf,PS10] = mapminmax(ULS);
 I2(:,10)=xf;
  
 PEhelp=PE;
 PE=PE';
 [yf,PS11]= mapminmax(PE);
 PE=yf;
 PE=PE';

%% /////////////////////////////////////////////FNN initial parameters//////////////////////////////////////
HiddenNodes=10;       %Number of hidden nodes
Inputs=10;
Outputs=1;
Dim=(Inputs+Outputs+1)*HiddenNodes;  %Dimension of particles in PSO
TrainingNO=1050;       %Number of training samples

%% ////////////////////////////////////////////////////////PSO/////////////////////////////////////////////
%Initial Parameters for PSO
noP=50;           %Number of particles
Max_iteration=350;%Maximum number of iterations
neighbors=20;
w=2.0;              %Inertia weight
wMax=0.9;         %Max inertia weight
wMin=0.5;         %Min inertia weight 
c1=2.1495; %.8;
c2=2.1495; %.8;
dt=.8; 

vel=zeros(noP,Dim); %Velocity vector
pos=zeros(noP,Dim); %Position vector

%////////Cognitive component///////// 
pBestScore=zeros(noP);
pBest=zeros(noP,Dim);
pBestMSE=zeros(noP);
%////////////////////////////////////

%////////Social component///////////
gBestScore=inf;
gBest=zeros(1,Dim);
gBestMSE=0;

lBestScore=ones(noP)*inf;
lBest=zeros(noP, Dim);
lBestMSE=0;
%///////////////////////////////////

ConvergenceCurve=zeros(1,Max_iteration); %Convergence vector

%Initialization
for i=1:size(pos,1) % For each Particle
    for j=1:size(pos,2) % For each dimension
           pos(i,j)=rand();
        vel(i,j)=0.3*rand();
    end
end

 %initialize gBestScore for min
 gBestScore=inf;
 
 PE_true = sum(PE == 1);
 PE_false = length(PE) - PE_true;
    
for Iteration=1:Max_iteration
    %Calculate MSE
    for i=1:size(pos,1)  
        for ww=1:((Inputs+Outputs)*HiddenNodes)
            Weights(ww)=pos(i,ww);
        end
        for bb=(Inputs+Outputs)*HiddenNodes+1:Dim
            Biases(bb-((Inputs+Outputs)*HiddenNodes))=pos(i,bb);
        end        
        fitness=0;
        fitnessTrue = 0;
        fitnessTrueCount=0;
        fitnessFalse = 0;
        fitnessFalseCount=0;
        fitnessMSE=0;
        for pp=1:TrainingNO
            actualvalue(i)=My_FNN(Inputs,HiddenNodes,Outputs,Weights,Biases,I2(pp,1),I2(pp,2), I2(pp,3),I2(pp,4),I2(pp,5),I2(pp,6),I2(pp,7), I2(pp,8),I2(pp,9),I2(pp,10));
            if(PE(pp)==1)
                fitnessTrue = fitnessTrue+((1-actualvalue(i))^2);
                fitnessTrueCount = fitnessTrueCount + 1;
                fitnessMSE = fitnessMSE+((1-actualvalue(i))^2); %*4.0;
            else
                fitnessFalse = fitnessFalse+((0-actualvalue(i))^2)*6;
                fitnessFalseCount = fitnessFalseCount + 1;
                fitnessMSE=fitnessMSE+((0-actualvalue(i))^2);%/PE_false;
            end
%             if(PE(pp)==-1)
%                 fitness=fitness+(1-actualvalue(i))^2;
%             end
            runningfitness(i)=fitness;
        end
        %fitness=fitness/TrainingNO;
        fitnessTrue = fitnessTrue^2 / fitnessTrueCount;
        fitnessFalse = fitnessFalse^2 / fitnessFalseCount;
        fitness = sqrt((fitnessTrue^2 + fitnessFalse^2))/2;
        fitnessMSE = fitnessMSE / TrainingNO;

        if Iteration==1
            pBestScore(i)=fitness;
            pBest(i,:)=pos(i,:);
            pBestMSE(i)=fitnessMSE;
        end
        
        if(pBestScore(i)>fitness)
            pBestScore(i)=fitness;
            pBest(i,:)=pos(i,:);
            pBestMSE(i)=fitnessMSE;
        end
        
        if(gBestScore>fitness)
            gBestScore=fitness;
            gBest=pos(i,:);
            gBestMSE=fitnessMSE;
        end
        
        % Do neighbor stuff here
        
        if(gBestScore==1)
            break;
        end
    end

    %Update the w of PSO
    % w=wMin-Iteration*(wMax-wMin)/Max_iteration;
    w=0.7;
    vLimit = 0.5;
    
    %Update the velocity and position of particles
    for i=1:size(pos,1)
        for j=1:size(pos,2)       
            vel(i,j)=dt*(w*rand()*vel(i,j)+c1*rand()*(pBest(i,j)-pos(i,j))+c2*rand()*(gBest(j)-pos(i,j)));
          %  vel(i,j) = max(vel(i,j), -vLimit);
          %  vel(i,j) = min(vel(1,j),  vLimit);
            pos(i,j)=pos(i,j)+vel(i,j);
          %  pos(i,j) = max(pos(i,j), -1);
          %  pos(i,j) = min(pos(i,j), 1);
        end
    end
    %ConvergenceCurve(1,Iteration)=gBestScore;
    ConvergenceCurve(1,Iteration)=gBestMSE;
    
    %disp(['PSO is training FNN (Iteration = ', num2str(Iteration),' ,MSE = ', num2str(gBestScore),')'])        
    disp(['PSO is training FNN (Iteration = ', num2str(Iteration),' GScore = ', num2str(gBestScore),...
        ' ,MSE = ', num2str(gBestMSE),')'])        
end

%% ///////////////////////Calculate the classification//////////////////////
        TPRrate=0;
        TFRrate=0;
        FPRrate=0;
        FFRrate=0;
        NumPos=0;
        NumNeg=0;
        Weights=gBest(1:(Inputs+Outputs)*HiddenNodes);
        Biases=gBest((Inputs+Outputs)*HiddenNodes+1:Dim);
         for pp=TrainingNO+1:size(PE,1)
            outactualvalue(pp-TrainingNO)=My_FNN(Inputs,HiddenNodes,Outputs,Weights,Biases,I2(pp,1),I2(pp,2), I2(pp,3),I2(pp,4),I2(pp,5),I2(pp,6),I2(pp,7), I2(pp,8),I2(pp,9),I2(pp,10));
            if(PE(pp)==1)
                NumPos=NumPos+1;
%                if (round(actualvalue))==1 
                if outactualvalue(pp-TrainingNO) > 0.5
                    TPRrate=TPRrate+1;
                end
            end
            if(PE(pp)==-1)
                NumNeg=NumNeg+1;
%                if (round(actualvalue))==0 
                if outactualvalue(pp-TrainingNO) <= 0.5
                    TFRrate=TFRrate+1;
                end
            end
        end
disp(['True Pos # = ', num2str(TPRrate)]);
disp(['Tot Pos # = ', num2str(NumPos)]);
disp(['True Pos Classification rate = ', num2str(100*TPRrate/NumPos)]);
disp(['False Pos # = ', num2str(NumNeg-TFRrate)]);       

disp(['True Neg # = ', num2str(TFRrate)]);
disp(['Tot Neg # = ', num2str(NumNeg)]);
disp(['True Neg Classification rate = ', num2str(100*TFRrate/NumNeg)]);
disp(['False Neg # = ', num2str(NumPos-TPRrate)]); 

ClassificationRate=((TPRrate+TFRrate)/(size(PE,1)-TrainingNO))*100;
disp(['Classification rate = ', num2str(ClassificationRate)]);

%% Draw the convergence curve
figure(1);
hold on;      
semilogy(ConvergenceCurve);
title(['Classification rate : ', num2str(ClassificationRate), '%']); 
xlabel('Iteration');
ylabel('MSE');
box on
grid on
axis tight
hold off;

figure(2);
plotconfusion(PEhelp(TrainingNO+1:size(PE,1))',outactualvalue(1:(size(PE,1)-TrainingNO)));