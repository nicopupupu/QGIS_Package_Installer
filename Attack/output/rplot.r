args <- commandArgs(trailingOnly = TRUE)
wd <- getwd()
wd <- paste(wd,args[1], sep="/")
setwd(wd)
print(wd)
file.names <- dir(wd, pattern=".dat")
for(i in 1:length(file.names)){
    w1 <- read.csv(paste(file.names[i], sep=""))
    rev(w1)
    true <- subset(w1, w1$step4 == 1)
    false <- subset(w1, w1$step4 == 2)
    png(paste("plots/", file.names[i], ".png", sep=""))
    plot(w1$message, w1$duration, col=c("red","blue")[w1$step4], xlab="message", ylab="duration (ns)", pch=".", cex=1)
#plot(w1$message, w1$duration)
#abline(lm(w1$duration