########################################################################
# Exports a single week for a given room
########################################################################
require(gdata)
library(lubridate)
########################################################################
# Load file
week <- read.xls("data/week06.xls", sheet = 1, header = TRUE)

r1          <- week$R1
time_col    <- week$time
orig_time   <- ymd_hms(time_col, tz = "Europe/London")
time        <- with_tz(orig_time, "Europe/Oslo")

# Amounts
sun_a <- r1[wday(time) == 1]
mon_a <- r1[wday(time) == 2]
tue_a <- r1[wday(time) == 3]
wed_a <- r1[wday(time) == 4]
thu_a <- r1[wday(time) == 5]
fri_a <- r1[wday(time) == 6]
sat_a <- r1[wday(time) == 7]

# Times
sun_t <- time[wday(time) == 1]
mon_t <- time[wday(time) == 2]
tue_t <- time[wday(time) == 3]
wed_t <- time[wday(time) == 4]
thu_t <- time[wday(time) == 5]
fri_t <- time[wday(time) == 6]
sat_t <- time[wday(time) == 7]

# Remove NA 
sun_a[is.na(sun_a)] <- 0
mon_a[is.na(mon_a)] <- 0
tue_a[is.na(tue_a)] <- 0
wed_a[is.na(wed_a)] <- 0
thu_a[is.na(thu_a)] <- 0
fri_a[is.na(fri_a)] <- 0
sat_a[is.na(sat_a)] <- 0

# Combine
week <- cbind(mon_a, tue_a, wed_a, thu_a, fri_a, sat_a, sun_a, mon_t)

# Export data
save(week, file = 'week06.RData')