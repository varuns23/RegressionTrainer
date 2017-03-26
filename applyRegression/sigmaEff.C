float sigmaEff(std::vector<float> v, float threshold, float& xmin, float& xmax){
  
  // threshold should be < 1. and > 0.
  
  float minwidth = 999999999.;
  float width;
  unsigned j;
  
  int total = v.size();
  //     std::cout << "vector size = " << total << std::endl;
  
  std::sort(v.begin(),v.end());
  
  unsigned i = 0 ;
  //   while (i != v.size()){
  //     std::cout << "i = " << i << " - v(i) = " << v[i] << std::endl;
  //     i++;
  //   }      
  
  int max = (int)(threshold * total);
  
  for(i = 0; i < (total-max); i++) {

    j = i+max;
    
    //    std::cout << "i = " << i << " - v(i) = " << v[i] << std::endl;
    //    std::cout << "j = " << j << " - v(j) = " << v[j] << std::endl;
    width = v[j] - v[i];
    //    std::cout << "width = " << width << std::endl;
    
    if(width < minwidth) {
      minwidth = width;
      xmin = v[i];
      xmax = v[j];
      //std::cout << "found a smaller range of size: " << minwidth << " ( " << xmin << " - " << xmax << ") including " << max << " events ( " << i << " - " << j <<  ")." << std::endl;
    }
  }
  
  return (minwidth/2.);
  
}

