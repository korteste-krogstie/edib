########################################################################
# Builds a list of 7 elements: sunday - saturday
# Each element is a vector of 288 cells
#   cells: the mean of given days for all weeks, 
#   exluded weeks with maximum amount lower than minimum_threshold
########################################################################
raw_data <- get(load("data/days.RData"))
minimum_threshold <- 5

means <- list()
for (day_num in 1:7) {

    raw_day <- data.frame(raw_data[day_num])
    week_list <- c()
    
    for (i in 5:15) { 
        week_num <- paste("week",i,sep="") 
        week <- subset(raw_day, select=week_num)
        if (max(week) <= minimum_threshold) next
        week_list <- c(week_list, week_num)
    } 
    day_strip <- subset(raw_day, select = c(week_list))
    day <- data.frame(rowMeans(day_strip))
    colnames(day) <- colnames(raw_day[1])
    means <- c(means, day)
}
save(means, file = 'means.RData')
########################################################################