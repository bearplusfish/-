/* Name: Jiayu Xiong
 * unikey: jxio5417
 */



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ARGC_COUNT 5
#define ARGV_HEX_DATA_COUNT 3

typedef u_int8_t Byte;

// printf "    " for space_count times
void add_space(int space_count) {
    for(int i = 0; i < space_count; i++) {
        printf("    ");
    }
}


// return check sum of Byte_array
Byte get_check_sum(Byte Byte_array[], int array_size) {
    Byte check_sum = 0;
    for(int i = 0; i < array_size; i++) {
        check_sum = check_sum ^ Byte_array[i];
    }
    return check_sum;
}



// check if a char is valid hex char, if true, return the value of it, else return -1
int is_valid_hex_char(char hex_char) {
    if (hex_char >= 'A' && hex_char <= 'F') {
        return hex_char - 'A' + 10;
    }
    if (hex_char >= 'a' && hex_char <= 'f') {
        return hex_char - 'a' + 10;
    }
    if (hex_char >= '0' && hex_char <= '9') {
        return hex_char-'0';
    }
    return -1;
}

int valid_hex_parameter(char* hex_data, int index) {        
    if (strlen(hex_data) != 4) {
        printf("Error: Argument for delimiter byte %d is not of the correct length\n", index);
        exit(1);
    }

    if (hex_data[0] != '0' || hex_data[1] != 'x') {
        printf("Error: Argument for delimiter byte %d does not begin with 0x\n", index);
        exit(1);
    }

    if (is_valid_hex_char(hex_data[2]) == -1 || is_valid_hex_char(hex_data[3]) == -1) {
        printf("Error: Argument for delimiter byte %d is not a valid hex value\n", index);
        exit(1);
    }
    return 0;
}

void parse_hex_value(Byte* delimiter, char** hex_delimiter) {
    for(int i = 0; i < ARGV_HEX_DATA_COUNT; i++) {
        delimiter[i] = is_valid_hex_char(hex_delimiter[i][2]) * 16 + is_valid_hex_char(hex_delimiter[i][3]);
    }
}

//
void valid_hex_parameters(char** hex_data) {
    for (int i = 0; i < ARGV_HEX_DATA_COUNT; i++) {
        valid_hex_parameter(hex_data[i], i);
    }
}

// delimiter should be a array of size 4, last one is check sum
void init_delimiter_and_check_sum(Byte* delimiter, char** hex_delimiter) {
    valid_hex_parameters(hex_delimiter);
    // set delimiter value
    parse_hex_value(delimiter, hex_delimiter);
    // set check sum
    delimiter[3] = get_check_sum(delimiter, ARGV_HEX_DATA_COUNT);
    // finish, print to output
    for (int i = 0; i < ARGV_HEX_DATA_COUNT; i++) {
        printf("Delimiter byte %d is: %d\n", i, delimiter[i]);
    }
    printf("Checksum is: %d\n\n", delimiter[3]);
}

int find_chunk_end(FILE* file, int chunk_begin_offset, Byte delimiter[]) {
    if (fseek(file, chunk_begin_offset, 0) != 0 ) {
        return chunk_begin_offset;
    }

    Byte data[4];
    int result_end = chunk_begin_offset;
    
    while (fread(&data, sizeof(Byte), 4, file)) {

        

        for (int i = 0; i < 4; i++) {
            if (i == 3) {
                fseek(file, chunk_begin_offset, 0);
                return result_end;
            }
            if (delimiter[i] != data[i]) {
                break;
            }
        }
        result_end += 1;

        fseek(file, result_end, 0);
    }
    fseek(file, chunk_begin_offset, 0);
    return result_end;
}



int swizzle_data(Byte cur_packet[]) {
    Byte packet_x, packet_y, packet_z, sw;
    sw = cur_packet[3];
    char* swizzle_type_describe; 

    switch (sw){
        case 1:
            
            packet_x = cur_packet[0];
            packet_y = cur_packet[1];
            packet_z = cur_packet[2];
            swizzle_type_describe = "XYZ";
            break;
        
        case 2:
            
            packet_x = cur_packet[0];
            packet_y = cur_packet[2];
            packet_z = cur_packet[1];
            swizzle_type_describe = "XZY";
            break;
        case 3:
            
            packet_y = cur_packet[0];
            packet_x = cur_packet[1];
            packet_z = cur_packet[2];
            swizzle_type_describe = "YXZ";
            break;
        case 4:
            
            packet_y = cur_packet[0];
            packet_z = cur_packet[1];
            packet_x = cur_packet[2];
            swizzle_type_describe = "YZX";
            break;
        case 5:
            
            packet_z = cur_packet[0];
            packet_x = cur_packet[1];
            packet_y = cur_packet[2];
            swizzle_type_describe = "ZXY";
            break;
        case 6:
            
            packet_z = cur_packet[0];
            packet_y = cur_packet[1];
            packet_x = cur_packet[2];
            swizzle_type_describe = "ZYX";
            break;
        
        default:
            add_space(2);
            printf("Ignoring packet. Swizzle byte was: %d but can only be between 1 and 6.\n", sw);
            return -1;
    }
    add_space(2);
    printf("Data before swizzle -> B0: %d, B1: %d, B2: %d\n", cur_packet[0], cur_packet[1], cur_packet[2]);

    add_space(2);
    printf("Swizzle: %s\n", swizzle_type_describe);

    // swizzle data
    cur_packet[0] = packet_x;
    cur_packet[1] = packet_y;
    cur_packet[2] = packet_z;

    add_space(2);
    printf("Data after swizzle -> X: %d, Y: %d, Z: %d\n", packet_x, packet_y, packet_z);

    return 0;
}

int check_cur_packet_valid(Byte cur_packet[], Byte pre_packet[]) {
    char xyz[3] = {'X', 'Y', 'Z'};

    for (int i = 0; i < 3; i++) {
        int diff = abs(pre_packet[i] - cur_packet[i]);
        if (diff > 25) {
            add_space(2);
            printf("Ignoring packet. %c: %d. Previous valid packet's %c: %d. %d > 25.\n", xyz[i], cur_packet[i], xyz[i], pre_packet[i], diff);
            return -1;
        }
    }
    return 0;
}

int deal_with_packet(int packet_id, Byte cur_packet[], Byte pre_packet[]) {
    int space_count = 1;
    add_space(space_count);
    printf("Packet: %d\n", packet_id);
    Byte check_sum = get_check_sum(cur_packet, 4);
    if (check_sum != cur_packet[4]) {
        add_space(2);
        printf("Ignoring packet. Checksum was: %d instead of %d.\n", check_sum, cur_packet[4]);
        return -1;
    }
    if (swizzle_data(cur_packet) != 0 ) {
        return -1;
    }

    if (packet_id != 0 && check_cur_packet_valid(cur_packet, pre_packet)) {
        return -1;
    }

    return 0;
}


// input file position currently at chunk_begin
int deal_with_chunk(int id, int chunk_begin, int chunk_end, FILE* input_file) {
    printf("Chunk: %d at offset: %d\n", id, chunk_begin);
    // printf("%d\n", offset_end);

    int space_count = 1;
    // check chunk size first
    if(((chunk_end - chunk_begin) % 5) != 0 ) {
        printf("Error: Chunk must be divisible by 5 bytes.\n");
        return -1;
    }
    if((chunk_end - chunk_begin)  > 640) {
        printf("Error: Chunk size exceeds the maximum allowable chunk size of 640 bytes.\n");
        return -1;
    }




    // start to parse each packet
    float chunk_average[] = {0, 0, 0};
    Byte cur_packet[5], pre_packet[5];
    int packet_counter = 0, valid_packet_counter = 0;
    for (;chunk_begin < chunk_end; chunk_begin += 5) {
        // read 5 bytes
        if (fread(&cur_packet, 1, 5*sizeof(Byte), input_file) != 5) {
            printf("Error: not enough bytes to read.\n");
            exit(1);
        }
        
        // will set xyz in cur_packet in order
        // return 0 if this is cur_packet is a valid packet
        if(deal_with_packet(packet_counter, cur_packet, pre_packet) == 0) {
            valid_packet_counter++;
            chunk_average[0] += cur_packet[0];
            chunk_average[1] += cur_packet[1];
            chunk_average[2] += cur_packet[2];
            memcpy(pre_packet, cur_packet, 5*sizeof(Byte));
        }
        packet_counter++;

    }


    add_space(space_count);
    printf("Chunk Average X: %.2f, Average Y: %.2f, Average Z: %.2f\n", \
        chunk_average[0]/valid_packet_counter, chunk_average[1]/valid_packet_counter, chunk_average[2]/valid_packet_counter);
    
    return 0;
}


int main(int argc, char** argv) {
    // check argument count
    if (argc < ARGC_COUNT) {
        printf("Error: Not enough command line arguments.\n");
        return -1; 
    } else if (argc > ARGC_COUNT) {
        printf("Error: Too many command line arguments.\n");
        return -1;
    }

    // check file exist
    FILE *input_file = fopen(argv[1], "rb");
    if (input_file == NULL) {
        printf("Error: File %s does not exist!\n", argv[1]);
        return -1;
    }

    // initial delimiter
    Byte delimiter[4];
    init_delimiter_and_check_sum(delimiter, &argv[2]);

    // start to parse chunk
    int chunk_id, chunk_begin, chunk_end;
    chunk_id = 0;
    chunk_begin = 0;
    chunk_end = 0;

    while (1) {
        chunk_end = find_chunk_end(input_file, chunk_begin, delimiter); // find current chunk's end index
        if (chunk_end == chunk_begin) {
            // if (chunk_begin == 0) {
            //     printf("Chunk: 0 at offset: 0\n");
            //     add_space(1);
            //     printf("No valid packets were found for this chunk.\n");
            // }
            break;
        } else {
            // chunk data offset include chunk_begin, not include chunk_end
            deal_with_chunk(chunk_id, chunk_begin, chunk_end, input_file);
            chunk_id += 1;
            chunk_begin = chunk_end + 4;
            chunk_end = chunk_begin;
            printf("\n");
        }
    }
    return 0;
}