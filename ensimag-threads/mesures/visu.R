
data= read.csv("../mesures/mesures.csv",sep=";")

library(ggplot2);

reftime= c(1:length(data$date))
for (i in 1:length(data$date)) {
    print(i);
    tmptime= NA;
    for ( j in 1:length(data$date) ) {
        if (data[j, "nthreads"] == 1
            && data[j, "nvilles"] == data[i, "nvilles"]
            && data[j, "graine"] == data[i, "graine"]) {
            tmptime= data[j,"temps"]
        }
    }
    reftime[i] = tmptime;
}


d = data.frame(nvilles=data$nvilles, graines=data$graine, nthreads=data$nthreads, temps=data$temps, coupures=data$coupures, tempsseq=reftime);

p = ggplot(d, aes(y=temps,x=nthreads)) + geom_line() + facet_grid(graines ~ nvilles)
p
ggsave("../mesures/temps.svg",width=2*par("din")[1])

p = ggplot(d, aes(y=temps*nthreads,x=nthreads)) + geom_line() + facet_grid(graines ~ nvilles)
p
ggsave("../mesures/travail.svg",width=2*par("din")[1])

p = ggplot(d, aes(y=tempsseq/temps,x=nthreads)) + geom_line() + facet_grid(graines ~ nvilles)
p
ggsave("../mesures/acceleration.svg",width=2*par("din")[1])

p = ggplot(d, aes(y=tempsseq/(temps*nthreads),x=nthreads)) + geom_line() + facet_grid(graines ~ nvilles)
p
ggsave("../mesures/efficacite.svg",width=2*par("din")[1])

