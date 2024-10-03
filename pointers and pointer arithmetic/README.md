Name:Jiayu Xiong
unikey:jxio5417


For example 1:
1. at 0, read 9 bytes, find a trailer at 5, return chunk end 5, move forward 9
2. obtain first chunk at byte 0-5, read 5 byte for 1 packet in total
3. move to offset 9, end program

For example 2:
1. at 0, read 19 byte, find a trailer at 15, return chunk end 15, move forward 19
2. obtain first chunk at byte 0-15, read 15 byte for 5 packet in total
3. move to offset 19
4. at 19, read 5 bytes, find end of this file, return 24
5. obtain second chunk at byte 19-24, read 5 byte for second chunk
6. end program