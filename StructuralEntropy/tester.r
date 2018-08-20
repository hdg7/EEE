#!/usr/local/bin/Rscript

library(caret)
library("fpc")
library(ROCR)
library(ipred)
library(rpart)
library(party)
library(e1071)

args <- commandArgs(TRUE)

model <- readRDS(args[1])
ind <- read.table(args[2],sep=';',header=FALSE)
goal <- as.numeric(args[3])

prediction <- predict(model,  newdata=ind)
probability <- unlist(treeresponse(model, ind), use.names=F)[goal+1]

print(prediction)
print(probability)
