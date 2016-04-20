########################################################################
# Builds a list of 7 elements: sunday - saturday
# Each element is a vector of 288 cells
#   cells: the mean of all days, exluded empty days
# NOTE: empty days are manually removed (for now) by inspecting a plot
########################################################################
library(graphics)
########################################################################
data <- get(load("data/days.RData"))
# 1 - time
# 2 - week5
# 3 - week6
# 4 - week7
# 5 - week8
# 6 - week9
# 7 - week10
# 8 - week11
# 9 - week12
# 10 - week13
# 11 - week14

monday_raw      <- data.frame(data[2])
tuesday_raw     <- data.frame(data[3])
wednesday_raw   <- data.frame(data[4])
thursday_raw    <- data.frame(data[5])
friday_raw      <- data.frame(data[6])
saturday_raw    <- data.frame(data[7])
sunday_raw      <- data.frame(data[1])

monday_strip      <- subset(monday_raw, select = c(week5,week9,week10,week11,week14))
tuesday_strip     <- subset(tuesday_raw, select = c(week5,week8,week9,week10,week11,week14))
wednesday_strip   <- subset(wednesday_raw, select = c(week5,week7,week8,week9,week10,week11,week13,week14))
thursday_strip    <- subset(thursday_raw, select = c(week8,week9,week10,week11,week13,week14))
friday_strip      <- subset(friday_raw, select = c(week7,week8,week9,week10,week11,week13,week14))
saturday_strip    <- subset(saturday_raw, select = c(week7,week8,week9,week10,week11,week12,week13,week14))
sunday_strip      <- subset(sunday_raw, select = c(week7,week8,week9,week10,week11,week12,week13,week14))

monday <- rowMeans(monday_strip)
tuesday <- rowMeans(tuesday_strip)
wednesday <- rowMeans(wednesday_strip)
thursday <- rowMeans(thursday_strip)
friday <- rowMeans(friday_strip)
saturday <- rowMeans(saturday_strip)
sunday <- rowMeans(sunday_strip)
########################################################################
means <- cbind(monday, tuesday, wednesday, thursday, friday, saturday, sunday)
save(means, file = 'means.RData')
########################################################################