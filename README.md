# 2048-bot
An expectiminimax tree based AI program solves the well-known game 2048.
===
This project was the final result when I attended [Taiwan 2048 Bot Contest](https://www.facebook.com/2048BotContest/) in 2014. I got Awarded for Excellent (5th place in 58 teams) at the end. You can view results of other teams from this [archived page](http://web.archive.org/web/20140707074432/http://2048-botcontest.twbbs.org:80/download/stats.htm).

Addition to [expectiminimax tree](https://en.wikipedia.org/wiki/Expectiminimax_tree), I also used [genetic algorithm](https://en.wikipedia.org/wiki/Genetic_algorithm) to get the weight of each feature. I trained 6 evolutions and selected the best performed one (evo3.data in my training). Note that related codes about training is not included in this project. Some hacks about bit manipulation is heavily used to speed up during the search procedure. For those interest in this part, please refer to the great book [Hacker's Delight](https://www.amazon.com/Hackers-Delight-2nd-Henry-Warren/dp/0321842685). 
