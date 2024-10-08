from little_battle import load_config_file
# Don't remove any comments in this file
folder_path = "./invalid_files/"
s = 'Invalid Configuration File: '

# Please create appropriate invalid files in the folder "invalid_files"
# for each unit test according to the comments below and
# then complete them according to the function name


def test_file_not_found():
    # no need to create a file for FileNotFound
    filename = folder_path + 'not_found.txt'
    try:
        load_config_file(filename)
    except Exception as e:
        assert isinstance(e, FileNotFoundError)
        assert e.args[1] == 'No such file or directory'


def test_format_error():
    # add "format_error_file.txt" in "invalid_files"
    filename = folder_path + 'format_error_file.txt'
    error_msg = s + 'format error!'
    try:
        load_config_file(filename)
    except Exception as e:
        assert isinstance(e, SyntaxError)
        # print(error_msg, e.args[0])
        assert e.args[0] == error_msg


def test_frame_format_error():
    # add "frame_format_error_file.txt" in "invalid_files"
    filename = folder_path + 'frame_format_error_file.txt'
    error_msg = s + 'frame should be in format widthxheight!'
    try:
        load_config_file(filename)
    except Exception as e:
        assert isinstance(e, SyntaxError)
        # print(error_msg, e.args[0])
        assert e.args[0] == error_msg


def test_frame_out_of_range():
    # add "format_out_of_range_file.txt" in "invalid_files"
    filename = folder_path + 'format_out_of_range_file.txt'
    error_msg = s + 'width and height should range from 5 to 7!'
    try:
        load_config_file(filename)
    except Exception as e:
        assert isinstance(e, ArithmeticError)
        assert e.args[0] == error_msg


def test_non_integer():
    # add "non_integer_file.txt" in "invalid_files"
    filename = folder_path + 'non_integer_file.txt'
    error_msg = s + 'Food contains non integer characters!'
    try:
        load_config_file(filename)
    except Exception as e:
        assert isinstance(e, ValueError)
        assert e.args[0] == error_msg


def test_out_of_map():
    # add "out_of_map_file.txt" in "invalid_files"
    filename = folder_path + 'out_of_map_file.txt'
    error_msg = s + 'Wood contains a position that is out of map.'
    try:
        load_config_file(filename)
    except Exception as e:
        assert isinstance(e, ArithmeticError)
        assert e.args[0] == error_msg


def test_occupy_home_or_next_to_home():
    # add two invalid files: "occupy_home_file.txt" and
    # "occupy_next_to_home_file.txt" in "invalid_files"
    filename = folder_path + 'occupy_home_file.txt'
    error_msg = s + 'The positions of home bases or the positions next to the home bases are occupied!'
    try:
        load_config_file(filename)
    except Exception as e:
        assert isinstance(e, ValueError)
        assert e.args[0] == error_msg


def test_duplicate_position():
    # add two files: "dupli_pos_in_single_line.txt" and
    # "dupli_pos_in_multiple_lines.txt" in "invalid_files"
    filename = folder_path + 'dupli_pos_in_single_line.txt'
    error_msg = s + 'Duplicate position (4, 2)!'
    try:
        load_config_file(filename)
    except Exception as e:
        assert isinstance(e, SyntaxError)
        assert e.args[0] == error_msg


def test_odd_length():
    # add "odd_length_file.txt" in "invalid_files"
    filename = folder_path + 'odd_length_file.txt'
    error_msg = s + 'Wood has an odd number of elements!'
    try:
        load_config_file(filename)
    except Exception as e:
        assert isinstance(e, SyntaxError)
        assert e.args[0] == error_msg


def test_valid_file():
    # no need to create file for this one, just test loading config.txt
    load_config_file('./config.txt')


# you can run this test file to check tests and load_config_file
if __name__ == "__main__":

    test_file_not_found()
    test_format_error()
    test_frame_format_error()
    test_frame_out_of_range()
    test_non_integer()
    test_out_of_map()
    test_occupy_home_or_next_to_home()
    test_duplicate_position()
    test_odd_length()
    test_valid_file()
