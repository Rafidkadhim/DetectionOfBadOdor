%% ladda in det intränade neuronätet

alex = alexnet;
layers = alex.Layers

%% Anpassa nätverket till x antal klasser, 4 i detta fall
layers(23) = fullyConnectedLayer(5); 
layers(25) = classificationLayer

%% Lägga upp träningsdatan. "Datset1 är en mapp med alla subfolders i.
%varje subfolder ha ca 800 bilder var på en specifik klass"
allImages = imageDatastore('Datset1', 'IncludeSubfolders', true, 'LabelSource', 'foldernames');
[trainingImages, testImages] = splitEachLabel(allImages, 0.8, 'randomize');



%% Träna nätverket igen, 20 cyklar, Epochs.
opts = trainingOptions('sgdm', 'InitialLearnRate', 0.001, 'MaxEpochs', 4, 'MiniBatchSize', 64); 
myNet = trainNetwork(trainingImages, layers, opts);

%% .Mäter noggranhet, mål är 100%
predictedLabels = classify(myNet, testImages); 
accuracy = mean(predictedLabels == testImages.Labels)
%% Kopplad kamera som klassificerar varje ram i videofilmen och skickar ut en label
while true   
    picture = camera.snapshot;              % Ta en bild    
    picture = imresize(picture,[227,227]);  % skala om bilden till 227x227

    label = classify(myNet, picture);        % Klassificera bilden
       
    image(picture);     % Visa bilden 
    title(char(label)); % Visa label/namn
    drawnow;   
end
