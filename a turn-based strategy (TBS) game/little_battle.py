import sys

s = 'Invalid Configuration File: '
# Please implement this function according to Section "Read Configuration File"


def load_config_file(filepath):
    # It should return width, height, waters, woods, foods, golds based on the file
    # Complete the test driver of this function in file_loading_test.py
    width, height = 0, 0
    waters, woods, foods, golds = [], [], [], []  # list of position tuples
    resources = [waters, woods, foods, golds]

    # file not exist
    f = open(filepath, 'r')

    # format error
    labels = ['Frame', 'Water', 'Wood', 'Food', 'Gold']
    i = 0
    lines = []
    for line in f.readlines():
        if i == 5:
            raise SyntaxError(s+'format error!')
        lines.append(line.strip())
        if not line.startswith(labels[i]+':'):
            raise SyntaxError(s+'format error!')
        i += 1

    if len(lines) != 5:
        raise SyntaxError(s+'format error!')

    # frame format error
    lst = lines[0].split()
    if len(lst) == 2:
        count = 0
        for item in lst[1].split('x'):
            count += 1
            if not item.isdigit():
                raise SyntaxError(
                    s + 'frame should be in format widthxheight!')
        if count != 2:
            raise SyntaxError(s + 'frame should be in format widthxheight!')
    else:
        raise SyntaxError(s + 'frame should be in format widthxheight!')

    # frame out range
    width = int(lst[1].split('x')[0])
    height = int(lst[1].split('x')[1])
    if 4 < width < 8 and 4 < height < 8:
        pass
    else:
        raise ArithmeticError(s + 'width and height should range from 5 to 7!')

    # check second to last line

    next_home = [(0, 1), (1, 0), (2, 1), (1, 2),
                 (width-3, height-2), (width-2, height-3),
                 (width-1, height-2), (width-2, height-1),
                 (1, 1), (width-2, height-2)]
    duplications = []
    i = 1
    for line in lines[1:]:
        label = labels[i]
        lst = line.split(':')[1].strip().split()

        for item in lst:
            if not item.isdigit():
                raise ValueError(
                    s + '%s contains non integer characters!' % label)
        if len(lst) % 2:
            raise SyntaxError(s + '%s has an odd number of elements!' % label)

        lst = [int(item) for item in lst]
        for j in range(0, len(lst), 2):
            x, y = lst[j], lst[j+1]
            if x >= width or y >= height:
                raise ArithmeticError(
                    s + '%s contains a position that is out of map.' % label)

            if (x, y) in next_home:
                raise ValueError(
                    s + 'The positions of home bases or the positions next to the home bases are occupied!')
            elif (x, y) in duplications:
                raise SyntaxError(s + 'Duplicate position (%s, %s)!' % (x, y))
            resources[i-1].append((x, y))
            duplications.append((x, y))
        i += 1

    print(f'Configuration file {filepath} was loaded.')
    return width, height, waters, woods, foods, golds


def display_price():
    # print()
    print('Recruit Prices:')
    print('  Spearman (S) - 1W, 1F')
    print('  Archer (A) - 1W, 1G')
    print('  Knight (K) - 1F, 1G')
    print('  Scout (T) - 1W, 1F, 1G')


def display_map(board):
    print('Please check the battlefield, commander.')
    width = len(board[0])
    height = len(board)
    print('  X' + ' '.join(['0'+str(i) for i in range(width)]) + 'X')
    print(' Y+' + '-'*(3*width-1) + '+')
    for j in range(height):
        print('0'+str(j) + '|' + '|'.join(board[j]) + '|')
    print(' Y+' + '-'*(3*width-1) + '+')


class Army:
    def __init__(self, name, x, y):
        self.name = name
        self.x = x
        self.y = y
        self.died = False

    def encounter(self, other):
        counters = {'Spearman': ['Knight', 'Scout'],
                    'Archer': ['Spearman', 'Scout'],
                    'Knight': ['Archer', 'Scout'],
                    'Scout': []}
        if self.name == other.name:
            self.died = True
            other.died = True
            print("We destroyed the enemy %d with massive loss!" % self.name)
        elif other.name in counters[self.name]:
            other.died = True
            print("Great! We defeated the enemy %s!" % other.name)
        else:
            self.died = True
            print('We lost the army %s due to your command!' % self.name)


class Player:
    def __init__(self, name, board):
        self.name = name
        self.board = board
        self.asset = [2, 2, 2]
        self.armies = []
        self.year = 617
        if self.name == 1:
            self.base = ((0, 1), (1, 0), (2, 1), (1, 2))
        else:
            width = len(board[0])
            height = len(board)
            self.base = ((width-3, height-2), (width-2, height-3),
                         (width-1, height-2), (width-2, height-1))

    def get_army(self, x, y):
        for army in self.armies:
            if army.x == x and army.y == y:
                return army

    def recruit_army(self, army_abbr):
        d = {'S': 'Spearman', 'K': 'Knight', 'A': 'Archer', 'T': 'Scout'}
        costs = {'S': [1, 1, 0], 'K': [0, 1, 1],
                 'A': [1, 0, 1], 'T': [1, 1, 1]}
        for i in range(3):
            if self.asset[i] < costs[army_abbr][i]:
                print('Insufficient resources. Try again.')
                return False
        while 1:
            print()
            choice = input(
                "You want to recruit a %s. Enter two integers as format ‘x y’ to place your army.\n" % d[army_abbr])
            if choice.count(' ') == 1:
                x = choice.split()[0]
                y = choice.split()[1]
                if x.isdigit() and y.isdigit():
                    x, y = int(x), int(y)
                    if (x, y) not in self.base or self.board[y][x] != '  ':
                        print(
                            "You must place your newly recruited unit in an unoccupied position next to your home base. Try again.")
                    else:
                        self.asset = [self.asset[i]-costs[army_abbr][i]
                                      for i in range(3)]
                        army = Army(d[army_abbr], x, y)
                        self.armies.append(army)
                        self.board[y][x] = army_abbr + str(self.name)
                        print('\nYou has recruited a %s.\n' % d[army_abbr])
                        print('[Your Asset: Wood - %d Food - %d Gold - %d]' %
                              tuple(self.asset))
                        return True
                else:
                    print('Sorry, invalid input. Try again.')

            elif choice == 'DIS':
                display_map(self.board)
            elif choice == 'PRIS':
                display_price()
            elif choice == 'QUIT':
                sys.exit()
            else:
                print('Sorry, invalid input. Try again.')

    def move_army(self, other, army, x1, y1, through=False, display=True):
        directions = [(0, 1), (0, -1), (1, 0), (-1, 0)]
        x0, y0 = army.x, army.y
        if self.board[y1][x1][1] == str(self.name) and not through:
            print('Invalid move. Try again.\n')
            return False
        elif (x0-x1, y0-y1) in directions:
            if display:
                print("\nYou have moved %s from (%d, %d) to (%d, %d)." %
                      (army.name, x0, y0, x1, y1))
            self.board[y0][x0] = '  '
            army.x = x1
            army.y = y1
            if self.board[y1][x1] == '~~':
                army.died = True
                self.armies.remove(army)
                print('We lost the army %s due to your command!' % army.name)
            elif self.board[y1][x1] == 'WW':
                self.asset[0] += 2
                print('Good. We collected 2 Wood.')
            elif self.board[y1][x1] == 'FF':
                self.asset[1] += 2
                print('Good. We collected 2 Food.')
            elif self.board[y1][x1] == 'GG':
                self.asset[2] += 2
                print('Good. We collected 2 Gold.')
            elif self.board[y1][x1] == 'H' + str(self.name % 2 + 1):
                print("The army %s captured the enemy’s capital." % army.name)
                name = input('What’s your name, commander?\n')
                print()
                print(
                    "***Congratulation! Emperor %s unified the country in %s.***" % (name, self.year))
                sys.exit()
            elif self.board[y1][x1][1] == str(self.name % 2 + 1):
                enemy = other.get_army(x1, y1)
                army.encounter(enemy)
                if enemy.died:
                    other.armies.remove(enemy)
                if army.died:
                    self.armies.remove(army)

            if not army.died:
                name = army.name[0] if army.name != 'Scout' else 'T'
                self.board[y1][x1] = name + str(self.name)
            if not through:
                print()
            return True
        else:
            print('Invalid move. Try again.\n')
            return False

    def move_scout(self, other, army, x1, y1):
        directions = [(0, 1), (0, -1), (1, 0), (-1, 0)]
        x0, y0 = army.x, army.y
        if self.board[y1][x1][1] == str(self.name):
            print('Invalid move. Try again.\n')
            return False
        elif (x0-x1, y0-y1) in directions:
            return self.move_army(other, army, x1, y1, through=False, display=True)

        elif ((x0-x1)//2, (y0-y1)//2) in directions:
            print("\nYou have moved %s from (%d, %d) to (%d, %d)." %
                  (army.name, x0, y0, x1, y1))
            mid_x, mid_y = (x0+x1) // 2, (y0+y1) // 2
            self.move_army(other, army, mid_x, mid_y,
                           through=True, display=False)
            if not army.died:
                self.move_army(other, army, x1, y1,
                               through=False, display=False)
            else:
                print()
            return True
        else:
            print('Invalid move. Try again.\n')
            return False


def display_armies(armies):
    kind = ['Spearman', 'Archer', 'Knight', 'Scout']
    # print()
    print('Armies to Move:')
    for name in kind:
        lst = []
        for army in armies:
            if army.name == name:
                lst.append(str((army.x, army.y)))
        if len(lst) > 0:
            print('  %s: ' % name + ', '.join(lst))
    print()


def walk_through():
    width, height, waters, woods, foods, golds = load_config_file(sys.argv[1])
    # width, height, waters, woods, foods, golds = load_config_file('config.txt')
    print('Game Started: Little Battle! (enter QUIT to quit the game)\n')
    board = [['  ' for i in range(width)] for j in range(height)]
    board[1][1] = 'H1'
    board[-2][-2] = 'H2'
    for (x, y) in waters:
        board[y][x] = '~~'
    for (x, y) in woods:
        board[y][x] = 'WW'
    for (x, y) in foods:
        board[y][x] = 'FF'
    for (x, y) in golds:
        board[y][x] = 'GG'
    display_map(board)
    print('(enter DIS to display the map)\n')
    display_price()
    print('(enter PRIS to display the price list)\n')
    year = 617
    index = 1
    players = [Player(1, board), Player(2, board)]
    directions = [(0, 1), (0, -1), (1, 0), (-1, 0)]

    # 5.c-d one turn
    while 1:
        player_next = players[index]
        index = (index + 1) % 2
        player = players[index]

        print('-Year %d-\n' % player.year)
        print("+++Player %d's Stage: Recruit Armies+++\n" % player.name)
        print('[Your Asset: Wood - %d Food - %d Gold - %d]' %
              tuple(player.asset))

        # recruit army stage
        while 1:
            # 5.d.0
            # check resources and places
            if sum(player.asset) < 2 or player.asset.count(0) >= 2:
                print('No resources to recruit any armies.')
                break
            home = (width-2, height-2) if index == 1 else (1, 1)
            empty_place = False
            for direction in directions:
                x, y = home[0]+direction[0], home[1]+direction[1]
                if board[y][x] == '  ':
                    empty_place = True
            if not empty_place:
                print('No place to recruit new armies.')
                break

            print()
            choice = input(
                "Which type of army to recruit, (enter) ‘S’, ‘A’, ‘K’, or ‘T’? Enter ‘NO’ to end this stage.\n")
            # 5.d.i
            if choice in ['S', 'A', 'K', 'T']:
                player.recruit_army(choice)

            elif choice == 'NO':
                break
            elif choice == 'DIS':
                display_map(board)
            elif choice == 'PRIS':
                display_price()
                # print()
            elif choice == 'QUIT':
                sys.exit()
            else:
                print('Sorry, invalid input. Try again.')

        # 5.e move army stage
        print("\n===Player %d's Stage: Move Armies===\n" % player.name)
        wait_move = [army for army in player.armies]
        # 5.f
        while len(wait_move) != 0:
            display_armies(wait_move)
            choice = input(
                "Enter four integers as a format ‘x0 y0 x1 y1’ to represent move unit from (x0, y0) to (x1, y1) or ‘NO’ to end this turn.\n")
            if choice.count(' ') == 3:
                x0 = choice.split()[0]
                y0 = choice.split()[1]
                x1 = choice.split()[2]
                y1 = choice.split()[3]
                if x0.isdigit() and y0.isdigit() and x1.isdigit() and y1.isdigit():
                    x0, y0 = int(x0), int(y0)
                    x1, y1 = int(x1), int(y1)
                    army = player.get_army(x0, y0)
                    if x0 == x1 and y0 == y1:
                        print("Invalid move. Try again.\n")
                    elif x0 <= width and y0 <= height and x1 <= width and y1 <= height:
                        if board[y1][x1][1] == str(player.name):
                            print("Invalid move. Try again.\n")
                        elif army and army in wait_move:
                            if army.name == 'Scout':
                                if player.move_scout(player_next, army, x1, y1):
                                    wait_move.remove(army)
                            else:
                                if player.move_army(player_next, army, x1, y1):
                                    wait_move.remove(army)

                    else:
                        print("Invalid move. Try again.\n")
                else:
                    print("Invalid move. Try again.\n")

            elif choice == 'NO':
                break
            elif choice == 'DIS':
                display_map(board)
                print()
            elif choice == 'PRIS':
                display_price()
            elif choice == 'QUIT':
                sys.exit()
            else:
                print("Invalid move. Try again.\n")

        # 5.f
        if len(wait_move) == 0:
            print("No Army to Move: next turn.\n")
            # break
        player.year += 1


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 little_battle.py <filepath>")
    # sys.exit()
    # width, height, waters, woods, foods, golds = load_config_file(sys.argv[1])
    # width, height, waters, woods, foods, golds = load_config_file('config.txt')
    # print('Game Started: Little Battle! (enter QUIT to quit the game)\n')
    walk_through()
