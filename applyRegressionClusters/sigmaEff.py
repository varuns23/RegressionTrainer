def sigmaEff2(v, thr):
    minwidth = 999999999.
    width = 0.
    
    j = 0
    
    total = len(v)
    data = sorted(v)
    max = int(thr * total);
  
    for i in xrange(total-max):

        j = i + max
        width = data[j] - data[i];
    
        if(width < minwidth):
            minwidth = width
            xmin = data[i]
            xmax = data[j]
  
  return (minwidth/2.);
  


