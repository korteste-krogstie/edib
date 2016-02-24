function[n] = eit_dim_num(filename)
    info=ncinfo(filename);
    n=info.Dimensions(1);
end

function[num] = eit_num(filename,i0,i1)
    num=ncread(filename,'num',i0,i1,1)
end