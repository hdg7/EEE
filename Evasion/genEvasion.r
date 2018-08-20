#!/usr/local/bin/Rscript
# /home/menendez/projects/AntiEnt/EnTS m.ts.u.VirusShare_efe55d4e3f3f1c1b7bb1150faef8e1e0.bin /home/menendez/config.ini /home/menendez/coevoData/EnTS/Test.model 10 5
library("GA")
library("proxy")
library("MASS")

args <- commandArgs(TRUE)
nameTechFolder <- args[1]
nameInd <- args[2]
nameExtraAdap <- args[3]
nameModel <- args[4]
typesNum <- as.numeric(args[5])
#fileSize <- as.numeric(args[6])
maxiter <- as.numeric(args[6])

fitness <- function(populus)
{
        populus[seq(1,typesNum*5,5)+22]<-round(populus[seq(1,typesNum*5,5)+22])
        populus[seq(1,22)]<-round(populus[seq(1,22)])
	populus[which(populus<0)] <- 0
       	value<-system(paste("sh /home/menendez/projects/AntiEnt/Evasion/fitness.sh",nameTechFolder,nameInd,nameExtraAdap,nameModel,as.character(typesNum),paste(as.character(populus),
               collapse=","),sep=" "),intern=TRUE)
        as.numeric(value)
}
#Falta meter, al principio, el orden de las secciones. Deben ser 12 números enteros del 1 al 4
min <-  c(0,0,0,0,0)
max <- c(20,1,1,1,1)
minsec <- c(rep(47,8),rep(1,13))
maxsec <- c(rep(125,8),rep(4,13))
minv <- c(0,minsec,rep(min,typesNum))
maxv <- c(300,maxsec,rep(max,typesNum))

GA <- ga(type="real-valued",fitness=fitness,min=minv,max=maxv,maxiter=maxiter)
saveRDS(GA,paste("genetic.",basename(nameInd),".rds",sep=""))

