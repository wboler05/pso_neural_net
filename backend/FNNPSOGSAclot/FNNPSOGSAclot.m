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
%% -----------------------------------------------------------------------------

clc
clear all
close all

%% ////////////////////////////////////////////////////Data set preparation/////////////////////////////////////////////
 in=xlsread('clean.xlsx');
 randomorder=randperm(size(in,1));
 x=in(randomorder,:);
 EtCO2=x(1:2101,2);
 Age=x(1:2101,3);
 SystBP=x(1:2101,4);
 SaO2=x(1:2101,5);
 HEM=x(1:2101,6);
 TOBAC=x(1:2101,7);
 SURG=x(1:2101,8);
 CARD=x(1:2101,9);
 DVT=x(1:2101,10);
 ULS=x(1:2101,11);
 PE=x(1:2101,1);
 
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
HiddenNodes=17;       %Number of hidden nodes
Inputs=10;
Outputs=1;
Dim=(Inputs+Outputs+1)*HiddenNodes;  %Dimension of particles in PSO
TrainingNO=1050;       %Number of training samples

%% ////////////////////////////////////////////////////////GSA/////////////////////////////////////////////

%Configurations and initializations

noP = 100;             %Number of masses
Max_iteration  = 25;  %Maximum number of iteration
w=2;              %Inertia weight
wMax=0.9;         %Max ineirtia weight
wMin=0.5;         %Min inertia weight

CurrentFitness =zeros(noP,1);

G0=1; %Gravitational constant
CurrentPosition = rand(noP,Dim); %Postition vector
Velocity = .3*randn(noP,Dim) ; %Velocity vector
acceleration=zeros(noP,Dim); %Acceleration vector
mass(noP)=0; %Mass vector
force=zeros(noP,Dim);%Force vector

%Vectores for saving the location and MSE of the best mass
gBestScore=inf;
gBest=zeros(1,Dim);


ConvergenceCurve=zeros(1,Max_iteration); %Convergence vector

%Main loop
Iteration = 0 ;                 
while  ( Iteration < Max_iteration )
    Iteration = Iteration + 1;  
    G=G0*exp(-20*Iteration/Max_iteration); %Equation (3.3)
    force=zeros(noP,Dim);
    mass(noP)=0;
    acceleration=zeros(noP,Dim);

%Calculate MSEs

 for i = 1:noP
        for ww=1:((Inputs+Outputs)*HiddenNodes)
            Weights(ww)=CurrentPosition(i,ww);
        end
        for bb=(Inputs+Outputs)*HiddenNodes+1:Dim
            Biases(bb-((Inputs+Outputs)*HiddenNodes))=CurrentPosition(i,bb);;
        end              
        fitness=0;
        for pp=1:TrainingNO
            actualvalue(i)=My_FNN(Inputs,HiddenNodes,Outputs,Weights,Biases,I2(pp,1),I2(pp,2), I2(pp,3),I2(pp,4),I2(pp,5),I2(pp,6),I2(pp,7), I2(pp,8),I2(pp,9),I2(pp,10));
            if(PE(pp)==1)
                fitness=fitness+((1-actualvalue(i))^2)*4.0;
            else fitness=fitness+((0-actualvalue(i))^2);
            end
        end
        fitness=fitness/TrainingNO; %Equation (5.4)
        CurrentFitness(i) = fitness;     
        
        if(gBestScore>fitness)
            gBestScore=fitness;
            gBest=CurrentPosition(i,:);
        end  
end

best=min(CurrentFitness);%Equation (3.10)
worst=max(CurrentFitness);%Equation (3.11)

for i=1:noP
    mass(i)=(CurrentFitness(i)-0.99*worst)/(best-worst);%Equation (3.9) 
end

for i=1:noP
    mass(i)=mass(i)*5/sum(mass);%Equation (3.14)  
    
end

%Calculate forces

for i=1:noP
    for j=1:Dim
        for k=1:noP
            if(CurrentPosition(k,j)~=CurrentPosition(i,j))
                %Equation (3.5)
                force(i,j)=force(i,j)+ rand()*G*mass(k)*mass(i)*(CurrentPosition(k,j)-CurrentPosition(i,j))/abs(CurrentPosition(k,j)-CurrentPosition(i,j));
                
            end
        end
    end
end

%Calculate a

for i=1:noP
       for j=1:Dim
            if(mass(i)~=0)
                acceleration(i,j)=force(i,j)/mass(i);%Equation (3.6)
            end
       end
end

%Update inertia weight

w=wMin-Iteration*(wMax-wMin)/Max_iteration;

%Calculate V
for i=1:noP
        for j=1:Dim
            %Equation (4.1)
            Velocity(i,j)=w*Velocity(i,j)+rand()*acceleration(i,j) + rand()*(gBest(j)-CurrentPosition(i,j));
        end
end

%Calculate X                                   
            
CurrentPosition = CurrentPosition + Velocity ; %Equation (4.2)

ConvergenceCurve(1,Iteration)=gBestScore; 
disp(['PSOGSA is training FNN (Iteration = ', num2str(Iteration),' ,MSE = ', num2str(gBestScore),')'])        
 
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
            actualvalue=My_FNN(Inputs,HiddenNodes,Outputs,Weights,Biases,I2(pp,1),I2(pp,2), I2(pp,3),I2(pp,4),I2(pp,5),I2(pp,6),I2(pp,7), I2(pp,8),I2(pp,9),I2(pp,10));
            if(PE(pp)==1)
                NumPos=NumPos+1;
                if (round(actualvalue))==1 
                    TPRrate=TPRrate+1;
                end
            end
            if(PE(pp)==-1)
                NumNeg=NumNeg+1;
                if (round(actualvalue))==0 
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
% hold on;      
% semilogy(ConvergenceCurve);
% title(['Classification rate : ', num2str(ClassificationRate), '%']); 
% xlabel('Iteration');
% ylabel('MSE');
% box on
% grid on
% axis tight
% hold off;
