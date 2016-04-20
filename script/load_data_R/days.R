########################################################################
# Builds a list of 7 elements: sunday - saturday
# Each element is a data-frame of 288 rows and 11 columns
#   rows: 00:01 - 23:56
#   cols: time + week 5 - 14
#       cells: time in the time-col, amount of people in the week cols
########################################################################
require(gdata)
library(lubridate)
########################################################################
file_list  <- list(5, 6, 7, 8, 9, 10, 11, 12, 13, 14)
day_names <- list("sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday")
clock = get(load("data/time.RData"))
data <- list()
########################################################################
for (day_num in 1:7){
    day <- data.frame(matrix(cbind(clock)))
    colnames(day)[1] <- day_names[day_num]
    for (week_num in file_list){
        input <- paste("data/week", week_num, ".xls", sep="")
        week <- read.xls(input, sheet = 1, header = TRUE)
        if (strtoi(week_num) > 12) {
            timezone <- "Europe/Helsinki"
        } else {
            timezone <- "Europe/Oslo"
        }
        time_col    <- week$time
        orig_time   <- ymd_hms(time_col, tz = "Europe/London")
        time        <- with_tz(orig_time, timezone)
        room        <- week$R1                                  # SET ROOM HERE
        week_day    <- room[wday(time) == day_num][1:288]
        week_day[is.na(week_day)] <- 0
        name        <- paste("week",week_num, sep="")
        day         <- cbind(day, cbind(week_day))
        colnames(day)[ncol(day)] <- name
    }
    data <- c(data, list(day))
}
########################################################################
save(data, file = 'days.RData')
########################################################################