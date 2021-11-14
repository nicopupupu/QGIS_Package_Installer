args <- commandArgs(trailingOnly = TRUE)
wd <- getwd()
wd <- paste(wd,args[1], sep="/")
setwd(wd)
print(wd)
file.names <- dir(wd, pattern=".dat")
for(i in 1:length(file.names)){
    w1 <- read.csv(paste(file.names[i], 