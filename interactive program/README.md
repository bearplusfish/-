# Name: Jiayu Xiong
# unikey: jxio5417




##How you store and access the movie data

* For the original string data, i used a get_clean_data() function to split it by '\n' and filter items whose length are smaller than 1, then i 
got a list, each item is info of a single movie.
* To access info of specific movie, i created a dict whose key is movie's name and value is its info, then i can easily access to movie info by its name
* If i need details of movie, i splited movie info by '. ' to get a list of details, then get detail by index.


##Describe how you have done error checking on the time data and how you have compared two different times

* A legal time input must be divided by ':' into two parts, both of them have two digits and can be converted to integers, besides, the first part is between 0 and 23, 
the second part is between 0 and 59. If illegal input detected, program end with error message.
* To compare two different times, i converted them to minutes by multiply hours(first part) by 60 and plus minutes(second part), than 
just compare the value, the smaller value means the earlier time.
* Both part of the input should have and only have two digits.


##List each of the idioms "search tasks" you have used in your solution
* In function 'movie_found', search key in dict.keys():

```
def movie_found(movie_name):
    movie_info_list = get_clean_data()
    keys = [info.split('. ')[0].lower() for info in movie_info_list]
    movie_dict = dict(zip(keys, movie_info_list))
    if movie_name in movie_dict.keys():
        return movie_dict[movie_name]
    return False

```

* In main function, to search different type of input, use for loop and compare values one by one: 

```
for o, a in opts:
    if o == '--show':
        if not (legal_time_format(a)):
            bye_with_msg(msg_bad_time_format)
        show_available_movies(a)
    if o == '--book':
        booking()
    if o == '--group':
        group_booking()

```







