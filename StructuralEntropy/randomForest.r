#!/usr/local/bin/Rscript

library(caret)
library("fpc")
library(ROCR)
library(rJava)
#####
#Arg1
#$P1 Train Ben
#$P2 Test Ben
#$P3 Train Mal
#$P4 Test Mal
#
#Arg2 Output Names
#Arg3 Classes
#Arg4 Names
####
args <- commandArgs(TRUE)

a <- read.table(args[1],sep=";",head=FALSE)
b <- a[,4:length(a[1,])]
rownames(b) <- a[,1]
colnames(b)[1] <- "Class"
b[,1] <- as.factor(b[,1])

correlations <- cor(b[,-1])
hcorrelations <- findCorrelation(correlations,cutoff=0.75)
save(hcorrelations,file=paste(args[2],"cor","RData",sep='.'))
b <- b[,-(hcorrelations+1)]
colnames(b)[-1] <- paste("V",5:(4+length(colnames(b)[-1])),sep="")

#Selecting Random Index
benignTrain <- which(a[,4]==0&a[,2]=="tr")
malwareTrain <- which(a[,4]==1&a[,2]=="tr")

benignTest <- which(a[,4]==0&a[,2]=="ts")
malwareTest <- which(a[,4]==1&a[,2]=="ts")

trainindex <- c(benignTrain,malwareTrain)
testindex <- c(benignTest,malwareTest)

trainindex <- sample(trainindex)
testindex <- sample(testindex)

trainset <- b[trainindex,]
testset <- b[testindex,]

#Naive Bayes
library(e1071)

library("RWeka")
library(party)
model <- cforest(as.factor(Class)~.,data=trainset,control = cforest_unbiased(ntree = 100))
#e <- evaluate_Weka_classifier(model,numFolds = 10, complexity = TRUE, class = TRUE)
prediction <- predict(model, newdata=testset[,-1])
tab <- table(pred = prediction, true = testset[,1])
mconf <- confusionMatrix(tab)
print("Random Forest")
#print(tab)
print(mconf$overall[1])

#rocr <- prediction(probabilities[,2],testset[,1])
#performance <- performance(rocr,"tpr","fpr")
#pem<-cbind(as.matrix(slot(performance,"x.values")[[1]]),as.matrix(slot(performance,"y.values")[[1]]),as.matrix(slot(performance,"alpha.values")[[1]]))
#write.table(file=paste("j48",args[2],".roc",sep=""),pem,col.names=FALSE,row.names=FALSE,sep=";")

#namesba <- rownames(spaceC[testindex,])
#classes <- cbind(namesba,as.character(prediction))
saveRDS(model,paste(args[2],"model",sep="."))
#write.table(file=paste(args[2],".j48.class",sep=""),classes,sep=";",row.names=FALSE)
write.table( paste("segmentation",rownames(b)[-hcorrelations],sep=""),
	file="segments.txt",row.names=FALSE,col.names=FALSE)
