function [ num ] = read_num( filename, idx, num )
    num=ncread(filename,'num',idx,num);
end