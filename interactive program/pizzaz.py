# Name: Jiayu Xiong
# unikey: jxio5417


import sys
import getopt

# data init and public parameters init
ori_data = "The Shining. 1980. 2h 26m. 10:00. Room 1\n\
Your Name. 2016. 1h 52m. 13:00. Room 1\n\
Fate/Stay Night: Heaven's Feel - III. Spring Song. 2020. 2h 0m. 15:00. Room 1\n\
The Night Is Short, Walk on Girl. 2017. 1h 32m. 17:30. Room 1\n\
The Truman Show. 1998. 1h 47m. 19:30. Room 1\n\
Genocidal Organ. 2017. 1hr 55m. 21:45. Room 1\n\
 \n\
Jacob's Ladder. 1990. 1h 56m. 10:00. Room 2\n\
Parasite. 2019. 2h 12m. 12:15. Room 2\n\
The Dark Knight. 2008. 2h 32min. 14:45. Room 2\n\
Blade Runner 2049. 2017. 2h 44m. 17:45. Room 2\n\
The Mist. 2007. 2h 6m. 21:00. Room 2\n\
Demon Slayer: Mugen Train. 2020. 1h59min. 23:20. Room 2\n\
 \n\
The Matrix. 1999. 2h 16m. 10:00. Room 3\n\
Inception. 2010. 2h 42m. 11:30. Room 3\n\
Shutter Island. 2010. 2h 19m. 14:30. Room 3\n\
Soul. 2020. 1hr 40m. 17:00. Room 3\n\
Mrs. Brown. 1997. 1h 41min. 19:00. Room 3\n\
Peppa Pig: Festival of Fun. 2019. 1h 8min. 21:00. Room 3\n\
Titanic. 1997. 3h 30min. 22:15. Room 3\n\
"
room_capacity = {'Room 1': 17, 'Room 2': 68, 'Room 3': 21}
popcorn_size_public_param = {'S': 'Small', 'M': 'Medium', 'L': 'Large'}
popcorn_price_public_param = {'S': 3.5, 'M': 5, 'L': 7, False: 0}
change_dict_public_param = {100: '100', 50: '50', 20: '20', 10: '10', 5: '5', 2: '2', 1: '1', 0.5: '50', 0.2: '20', 0.1: '10', 0.05: '5'}
legal_args = ['--show', '--book', '--group']
msg_wrong_switch = 'Sorry. This program does not recognise the switch options.\n'
msg_bad_time_format = 'Sorry. This program does not recognise the time format entered.\n'


# functions init
def greeting():
    print("-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\
~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\
~ Welcome to Pizzaz cinema ~\n\
~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\
-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\
")


def bye():
    print('Bye.')
    sys.exit(0)


def bye_with_msg(msg):
    if msg != '':
        print(msg)
    bye()


def get_clean_data():
    return [info for info in ori_data.split('\n') if len(info) > 1]


def legal_time_format(time):
    time = time.split(':')
    if len(time) != 2:
        return False
    if not time[0].isdigit() or not time[1].isdigit():
        return False
    if int(time[0]) < 0 or int(time[0]) > 23:
        return False
    if int(time[1]) < 0 or int(time[1]) > 59:
        return False
    if len(time[0]) != 2 or len(time[1]) != 2:
        return False
    return True


def get_minutes_of_time(time):
    time = time.split(':')
    return int(time[0]) * 60 + int(time[1])


def first_time_earlier(current_movie_time, time_input):
    if get_minutes_of_time(current_movie_time) < get_minutes_of_time(time_input):
        return True
    else:
        return False


def movie_found(movie_name):
    movie_info_list = get_clean_data()
    keys = [info.split('. ')[0].lower() for info in movie_info_list]
    movie_dict = dict(zip(keys, movie_info_list))
    if movie_name in movie_dict.keys():
        return movie_dict[movie_name]
    return False


def ask_name_of_movie():
    movie_name = input('What is the name of the movie you want to watch? ')
    found = movie_found(movie_name.lower())
    while not found:
        response = input('Sorry, we could not find that movie. Enter Y to try again or N to quit. ')
        while response.upper() != 'N' and response.upper() != 'Y':
            response = input('Sorry, we could not find that movie. Enter Y to try again or N to quit. ')

        if response.upper() == 'Y':
            movie_name = input('What is the name of the movie you want to watch? ')
            found = movie_found(movie_name.lower())

        if response.upper() == 'N':
            bye()

    if found:
        return found


def whether_order_popcorn():
    order_popcorn = input('\nWould you like to order popcorn? Y/N ')
    while order_popcorn.upper() != 'Y' and order_popcorn.upper() != 'N':
        order_popcorn = input('Would you like to order popcorn? Y/N ')

    if order_popcorn.upper() == 'Y':
        size = input('You want popcorn. What size Small, Medium or Large? (S/M/L) ')
        while size.upper() != 'S' and size.upper() != 'M' and size.upper() != 'L':
            size = input('You want popcorn. What size Small, Medium or Large? (S/M/L) ')
        return size.upper()

    if order_popcorn.upper() == 'N':
        return False


def whether_order_popcorn_for_each_person(people_number):
    res = [0] * people_number
    print('')
    for i in range(people_number):
        person_serial_number = i + 1
        order_popcorn = input('For person {}, would you like to order popcorn? Y/N '.format(person_serial_number))
        while order_popcorn.upper() != 'Y' and order_popcorn.upper() != 'N':
            order_popcorn = input('For person {}, would you like to order popcorn? Y/N '.format(person_serial_number))

        if order_popcorn.upper() == 'Y':
            size = input('Person {} wants popcorn. What size Small, Medium or Large? (S/M/L) '.format(person_serial_number))
            while size.upper() != 'S' and size.upper() != 'M' and size.upper() != 'L':
                size = input(
                    'Person {} wants popcorn. What size Small, Medium or Large? (S/M/L) '.format(person_serial_number))
            res[i] = size.upper()

        if order_popcorn.upper() == 'N':
            res[i] = False
    return res


def room_overload(movie_info, people_number):
    if people_number > room_capacity[movie_info.split('. ')[-1]]:
        return True
    return False


def people_number_exceeded_and_ask_again(people_num, capacity):
    response = input('Sorry, we do not have enough space to hold {} people in the theater room of {} seats. Enter Y to try a different movie name or N to quit.'.format(people_num, capacity))
    while response.upper() != 'N' and response.upper() != 'Y':
        response = input('Sorry, we do not have enough space to hold {} people in the theater room of {} seats. Enter Y to try a different movie name or N to quit.'.format(people_num, capacity))
    if response.upper() == 'Y':
        movie_info = ask_name_of_movie()
        return ask_group_booking_people_number(movie_info)
    if response.upper() == 'N':
        bye()


def ask_group_booking_people_number(movie_info):
    people_num = input('\nHow many persons will you like to book for? ')
    people_num = int(people_num)
    while people_num < 2:
        response = input('Sorry, you must have at least two customers for a group booking. Enter Y to try again or N to quit.')
        while response.upper() != 'Y' and response.upper() != 'N':
            response = input('Sorry, you must have at least two customers for a group booking. Enter Y to try again or N to quit.')

        if response.upper() == 'Y':
            people_num = input('How many persons will you like to book for? ')
            people_num = int(people_num)
            if room_overload(movie_info, people_num):
                return people_number_exceeded_and_ask_again(people_num, room_capacity[movie_info.split('. ')[-1]])
            return people_num

        if response.upper() == 'N':
            bye()
    if room_overload(movie_info, people_num):
        return people_number_exceeded_and_ask_again(people_num, room_capacity[movie_info.split('. ')[-1]])
    return people_num


def get_movie_price(movie_info):
    if first_time_earlier(movie_info.split('. ')[-2], '16:00'):
        return 13, 'before 16:00'
    else:
        return 15, 'from 16:00'


def get_popcorn_price(order_popcorn):
    return [popcorn_price_public_param[x] for x in order_popcorn]


def calculate_price_and_itemise_details(movie_info, order_popcorn, people_num=1):
    price_movie, type_movie = get_movie_price(movie_info)
    price_popcorn = get_popcorn_price(order_popcorn)
    total_cost = price_movie * people_num + sum(price_popcorn)
    ticket_discount_rate, popcorn_discount_rate = get_group_discount(total_cost)
    ticket_discount = price_movie * people_num - format_cost_after_discount(price_movie * people_num * (1 - ticket_discount_rate))
    popcorn_discount = sum(price_popcorn) - format_cost_after_discount(sum(price_popcorn) * (1 - popcorn_discount_rate))
    cost_after_discount = total_cost - ticket_discount - popcorn_discount
    print('')
    if people_num == 1:
        print('For {} person, the initial cost is '.format(people_num).ljust(34, ' ') + '$' + '{:.2f}'.format(total_cost).rjust(5, ' '))
    else:
        print('For {} persons, the initial cost is '.format(people_num).ljust(34, ' ') + '$' + '{:.2f}'.format(total_cost).rjust(5, ' '))

    for i in range(people_num):
        people_serial_number = i + 1
        print(' Person {}: Ticket {}'.format(people_serial_number, type_movie).ljust(34, ' ') + '$' + '{:.2f}'.format(price_movie).rjust(5, ' '))

        if price_popcorn[i] != 0:
            print(' Person {}: {} popcorn'.format(people_serial_number,popcorn_size_public_param[order_popcorn[i]]).ljust(34, ' ') + '$' + '{:.2f}'.format(popcorn_price_public_param[order_popcorn[i]]).rjust(5, ' ') + '\n')
    if ticket_discount + popcorn_discount < 1e-5:
        print(' No discounts applied'.ljust(34, ' ') + '$' + '{:.2f}'.format(0).rjust(5, ' ') + '\n')
        print('The final price is'.ljust(34, ' ') + '$' + '{:.2f}'.format(round(total_cost, 2)).rjust(5, ' ') + '\n')
        return round(total_cost, 2)
    elif people_num > 1:
        print(' Discount applied tickets x{}'.format(people_num).ljust(33, ' ') + '-$' + '{:.2f}'.format(ticket_discount).rjust(5, ' ') + '\n')
        print(' Discount applied popcorn x{}'.format(len([x for x in price_popcorn if x != 0])).ljust(33,' ') + '-$' + '{:.2f}'.format(popcorn_discount).rjust(5, ' ') + '\n')
        print('The final price is'.ljust(34, ' ') + '$' + '{:.2f}'.format(cost_after_discount).rjust(5, ' ') + '\n')
        return cost_after_discount


def format_cost_after_discount(cost):
    cost = round(cost, 2)
    digit_list = [c for c in str(cost)]
    if digit_list[-1] in ['0', '1', '2']:
        digit_list[-1] = '0'
        return float(''.join(digit_list))
    elif digit_list[-1] in ['8', '9']:
        residual = 0.02 if digit_list[-1] == '8' else 0.01
        return float(''.join(digit_list)) + residual
    else:
        digit_list[-1] = '5'
        return float(''.join(digit_list))


def print_seat_for_group_person(people_num):
    print('')
    for i in range(people_num):
        people_num = i + 1
        print('The seat number for person {} is #{}'.format(people_num, 2 * people_num - 1))


def get_group_discount(total_price):
    if total_price > 100:
        return 0.1, 0.2
    return 0, 0


def give_change(total_change):
    change_dict = {}
    print('Change: ${:.2f}'.format(total_change))
    for denomination in change_dict_public_param.keys():
        if total_change > 1e-6:
            number_of_this_denomination = total_change // denomination
            total_change = total_change - number_of_this_denomination * denomination
            if number_of_this_denomination != 0:
                change_dict[denomination] = int(number_of_this_denomination)

    blank = 3 if 100 in change_dict.keys() else 2
    for key in change_dict.keys():
        if key >= 1:
            print(' $' + '{}'.format(change_dict_public_param[key]).rjust(blank, ' ') + ': ' + str(change_dict[key]))
        else:
            print(' {}'.format(change_dict_public_param[key]).rjust(blank, ' ') + 'c: ' + str(change_dict[key]))
    print('')


def transact_and_give_change(cash_need_to_pay):
    cash_got = input('Enter the amount paid: $')
    total_change = float(cash_got) - cash_need_to_pay
    while total_change < 0 or (round(total_change / 0.05) - total_change / 0.05) > 1e-5:
        if total_change < 0:
            print('The user is ${:.2f} short. Ask the user to pay the correct amount.'.format(abs(total_change)))
        else:
            print('The input given is not divisible by 5c. Enter a valid payment.')
        cash_got = input('Enter the amount paid: $')
        total_change = float(cash_got) - cash_need_to_pay
    give_change(total_change)


def show_available_movies(time):
    movie_info_list = get_clean_data()
    for movie_info in movie_info_list:
        if first_time_earlier(time, movie_info.split('. ')[-2]):
            print(movie_info)
    print('')
    bye()


def booking():
    movie_info = ask_name_of_movie()
    popcorn_size = whether_order_popcorn()
    print('\nThe seat number for person 1 is #17')
    cost = calculate_price_and_itemise_details(movie_info, [popcorn_size])
    transact_and_give_change(cost)
    bye()


def group_booking():
    movie_info = ask_name_of_movie()
    people_num = ask_group_booking_people_number(movie_info)
    popcorn_size_for_group = whether_order_popcorn_for_each_person(people_num)
    print_seat_for_group_person(people_num)
    cost = calculate_price_and_itemise_details(movie_info, popcorn_size_for_group, people_num)
    transact_and_give_change(cost)
    bye()


def main():
    greeting()
    if len(sys.argv) == 1:
        sys.exit('Usage: python3 pizzaz.py [ --show <timenow> | --book | --group ]')
    argv = [x.lower() for x in sys.argv[1:]]
    try:
        opts, args = getopt.getopt(argv, '', ['show=', 'book', 'group'])
    except getopt.GetoptError:
        bye_with_msg(msg_wrong_switch)
    if len(args) != 0:
        bye_with_msg(msg_wrong_switch)

    for o, a in opts:
        if o == '--show':
            if not (legal_time_format(a)):
                bye_with_msg(msg_bad_time_format)
            show_available_movies(a)
        if o == '--book':
            booking()
        if o == '--group':
            group_booking()


if __name__ == '__main__':
    main()

