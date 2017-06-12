./scripts/feeds update -a
./scripts/feeds install -a
./feeds/puppies/rom/scripts/update_source.sh
#make clean
./feeds/puppies/rom/scripts/make.sh FAP_qca9531_ylxd_A275E make -j4
# V=s FORCE_UNSAFE_CONFIGURE=1 
