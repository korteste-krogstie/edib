function [ X, Y, T ] = read_ts( filename, trace_idx, idx, num )
    X = ncread(filename,'xpos',[trace_idx, idx],[1, num]);
    Y = ncread(filename,'ypos',[trace_idx, idx],[1, num]);
    T = ncread(filename,'tpos',[trace_idx, idx],[1, num]);
end