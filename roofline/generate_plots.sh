cd bw
    python3 generate_bw.py
    source generate_bw.sh
cd ..

for dir in $(ls -d */)
do
    if [[ "$dir" != "bw/" ]];then
        echo $dir
        
        cd $dir
        
        echo $(pwd)
        rm -rf build
        mkdir -p build
        
        cmake -S . -B ./build -GNinja -DCMAKE_BUILD_TYPE=Release > /dev/null
        
        cd build
        ninja
        cd ..
        build/exe --l1=$SIMD_L1_SIZE --l2=$SIMD_L2_SIZE

        rm -rf build *.log

        cd ..
    fi
done
python3 generate_plots.py