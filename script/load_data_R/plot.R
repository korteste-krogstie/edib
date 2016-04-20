########################################################################
# Plots data for all days
# Includes weeks where maximum amount is above minimum_threshold
########################################################################
library(graphics)
############################## WEEKS ###################################
raw_data <- get(load("data/days.RData"))
minimum_threshold <- 5

for (day_num in 1:7) {

    day <- data.frame(raw_data[day_num])
    time <- strptime(day[,1], "%H:%M", tz="")
    number_of_weeks <- length(day[-1])
    xrange <- range(0, 288) 
    yrange <- range(0, max(day[-1]))
    colors <- rainbow(number_of_weeks) 
    linetype <- seq(1,1,length.out=number_of_weeks)
    week_list <- list()
    
    plot(time, seq(0, max(day[-1]), length.out = 288), type="n", xlab="time", ylab="people")
    grid(nx=26)
    
    for (i in 5:15) { 
        week_num <- paste("week",i,sep="") 
        week <- subset(day, select=week_num)
        if (max(week) <= minimum_threshold) next
        week_list <- c(week_list, week_num)
        lines(time, week[,1], type="l", lwd=1, lty=1, col=colors[i]) 
    } 
    
    title(colnames(day)[1])
    x <- strptime("01:00", "%H:%M", tz="")
    y <- max(day[-1])-10
    legend(x,y, week_list, col=colors, lty=linetype, title="Week")

}
############################## MEANS ###################################
raw_means <- get(load("data/means.RData"))
clock <- get(load("data/clock.RData"))
minimum_threshold <- 5
time <- strptime(clock, "%H:%M", tz="")
means <- data.frame(raw_means)

number_of_days <- length(means)
colors <- rainbow(number_of_days) 
linetype <- seq(1,1,length.out=number_of_days)
day_list <- list()

plot(time, seq(0, max(means), length.out = 288), type="n", xlab="time", ylab="people")
grid(nx=26)

for (day_num in 1:length(means)) {
    if (max(means[,day_num]) <= minimum_threshold) next
    day_list <- c(day_list, names(means[day_num]))
    lines(time, means[,day_num], type="l", lwd=1, lty=1, col=colors[day_num]) 
}
title("Means")
x <- strptime("01:00", "%H:%M", tz="")
y <- max(means)-10
legend(x,y, day_list, col=colors, lty=linetype, title="Day")
