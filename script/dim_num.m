function[n] = dim_num(filename)
    info=ncinfo(filename);
    n=info.Dimensions(1);
end