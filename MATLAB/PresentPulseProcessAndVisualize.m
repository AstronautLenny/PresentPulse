%Leonard Farrell
%Created 11-30-2023
%Revised 12-01-2023

%Road Map:
%  1. Load raw pulse and flex data
%  2. (Figure 1) Plot raw pulse and flex data on separate subplots
%  3. (Figure 2) Plot raw pulse data, filtered pulse data, and beats spotted
%  4. (Figure 3) Plot approximated BPM and centerline
%  5. (Figure 4) Plot BPM centerlines for 3 rest and meditation trials

clc
clear
close all

%-------------(Figure 4) Plot centerlines for each dataset--------------
filename='LOGREST.xlsx';
fig1=0;
fig2=1;
fig3=1;
[restCenterline, tBPMrest] = getBPMCenterline(filename, fig1, fig2, fig3);

[meditateCenterline, tBPMmeditate] = getBPMCenterline('LOGMEDITATE.xlsx',0,0,0);
[activeCenterline, tBPMactive] = getBPMCenterline('LOGACTIVE.xlsx',0,0,0);

figure(4)
hold on
plot(tBPMrest,restCenterline)
plot(tBPMmeditate,meditateCenterline)
plot(tBPMactive,activeCenterline)
xlabel('time (s)')
ylabel('BPM (Beats per minute)')
title('BPM Approximations from pulse data collection sessions')
legend('rest','meditate','active')
set(gca,'Color',[1.00, 0.898, 0.706])
grid on
%--------------------------------------------------------------------------

function [yBPM,tBPM] = getBPMCenterline(filename, fig1, fig2, fig3)

fprintf('\n')
fprintf(filename)
fprintf('\n')
%--------------------------Load Data---------------------------------------
data=table2array(readtable(filename,'ReadVariableNames',false));
time=data(:,1)/1000;    %Timestamps from milliseconds to seconds
IRValue=data(:,2);      %IR reading, representing change in blood volume
%flexIndex=data(:,3);
%flexMiddle=data(:,4);
%--------------------------------------------------------------------------
%
%
%-------(Figure 1) Plot raw pulse and flex data on separate subplots-------
if(fig1==1)
    figure(1)
    
    subplot(2,1,1)
    ax1=gca;
    set(gca,'Color',[1.00, 0.898, 0.706])
    xlabel('time (s)')
    ylabel('Analog Value')
    title('Flex Sensor Data')
    legend('Index Finger', 'Middle Finger')
    grid on

    indexThresh=ones(length(time),1)*400;
    middleThresh=ones(length(time),1)*310;
    hold on
    plot(time,flexIndex,'.b')
    plot(time,flexMiddle,'r')
    plot(time,indexThresh,'--b')
    plot(time,middleThresh,'--r')
    hold off
    
    subplot(2,1,2)
    ax2=gca;
    set(gca,'Color',[1.00, 0.898, 0.706])
    xlabel('time (s)')
    ylabel('IR Value')
    title('Raw Pulse Sensor Data')    
    grid on

    plot(time,IRValue)   

    sgtitle('Raw Flex and Pulse Sensor Data')
    
    %{
    for i=1:length(time)
        boxWidth=100;
        flexMins=[min(flexIndex(i:i+boxWidth)) min(flexMiddle(i:i+boxWidth))];
        flexMaxes=[max(flexIndex(i:i+boxWidth)) max(flexMiddle(i:i+boxWidth))];

        flexYMin=min(flexMins);
        flexYMax=max(flexMaxes);
        pulseYMin=min(IRValue(i:i+boxWidth));
        pulseYMax=max(IRValue(i:i+boxWidth));

        if(flexYMin-10>320)
            flexYMin=310;   
        elseif(flexYMax+10<410)
            flexYMax=400;
        end
   
        axis(ax1,[time(i) time(i+boxWidth) flexYMin-10 flexYMax+10])   
        axis(ax2,[time(i) time(i+boxWidth) pulseYMin-100 pulseYMax+100])
        pause(0.0001);
    end
    %}
end
%--------------------------------------------------------------------------
%
%
%---------------Create ButterWorth bandpass filter for Pulse Data----------
%Algorithm for Butterworth filter application created with Dr. Compere
filtOrder=20;
filtSampleFreq=50; %(Hz)
freqBandLo=0.5; %(Hz)
freqBandHi=5; %(Hz)

[z,p,k] = butter(filtOrder/2,[freqBandLo freqBandHi]/(filtSampleFreq/2)); % zeros, poles, gain
sosPulse = zp2sos(z,p,k); % Convert zero-pole-gain filter parameters to second-order sections form
%--------------------------------------------------------------------------
%
%
%-------------------------Filter Pulse Data--------------------------------
dataIn=IRValue;
pulseDataOut = sosfilt(sosPulse,dataIn);
%--------------------------------------------------------------------------
%
%
%---------------------Beat Spotting Threshold Method-----------------------
yThresh=0;
aboveThresh=false;
count= 0;

%for i=1000:length(time)
for i=1:length(time)
    if pulseDataOut(i)>yThresh && ~aboveThresh
        count=count+1;
        tCaptureThresh(count)=time(i);
        yCaptureThresh(count)=pulseDataOut(i);
        aboveThresh=true;    
    end
    if pulseDataOut(i)<=yThresh
        aboveThresh=false;
    end
end

beatTime=tCaptureThresh(length(tCaptureThresh))-tCaptureThresh(1);
threshBPM=(count/beatTime)*60; %Beats per minute
fprintf('Average BPM with threshold method: %.2f\n',threshBPM)
%--------------------------------------------------------------------------
%
%
%-------------------Beat Spotting Slope Method-----------------------------
aboveThresh=false;
count= 0;

%for i=1000:length(time)-1
for i=1:length(time)-1
    if pulseDataOut(i+1)-pulseDataOut(i)<0 && ~aboveThresh
        count=count+1;
        tCaptureSlope(count)=time(i);
        yCaptureSlope(count)=pulseDataOut(i);
        aboveThresh=true;    
    end
    if pulseDataOut(i+1)-pulseDataOut(i)>0
        aboveThresh=false;
    end
end

beatTimeSlope=tCaptureSlope(length(tCaptureSlope))-tCaptureSlope(1);
slopeBPM=(count/beatTimeSlope)*60; %Beats per minute
fprintf('Average BPM with slope method: %.2f\n',slopeBPM)
%--------------------------------------------------------------------------
%
%
%--(Figure 2) Plot raw pulse data, filtered pulse data, and beats spotted--
if(fig2==1)
    figure(2)
    hold on
    plot(time,dataIn-13.5e4,'.-','Color',[169/255 169/255 169/255]) % original signal
    plot(time,pulseDataOut,'k.-') % filtered version
    plot(tCaptureThresh,yCaptureThresh,'b.','MarkerSize',20)
    plot(tCaptureSlope,yCaptureSlope,'r.','MarkerSize',20)
    plot(time,zeros(length(time),1),'k')
    xlabel('time (s)')
    ylabel('signal')
    grid on
    legend('dataIn, Input signal', 'dataOut, Filtered Signal','Beat w/ Threshold Method', 'Beat w/ Slope Method','Location','best') 
    set(gca,'Color',[1.00, 0.898, 0.706])
    grid on
end
%--------------------------------------------------------------------------
%
%
%----------------BPM Calculation with Moving Average---------------------
%Moving average technique inspired by Joey
N=150; %Window Size
for i=1:length(tCaptureSlope)-N
    yBPM(i)=(N/(tCaptureSlope(i+N)-tCaptureSlope(i)))*60;
    tBPM(i)=tCaptureSlope(i+N);            
end

fprintf('Average BPM with moving average method: %.2f\n',mean(yBPM));
%--------------------------------------------------------------------------
%
%
%-------------(Figure 3) Plot approximated BPM centerlines--------------
if(fig3==1)
    figure(3)
    plot(tBPM,yBPM)
    title('Approximated Rest BPM : Window Size = 150')
    xlabel('time (s)')
    ylabel('BPM')
    set(gca,'Color',[1.00, 0.898, 0.706])
    grid on
end
%--------------------------------------------------------------------------
end