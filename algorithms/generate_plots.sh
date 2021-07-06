cwd_=$(pwd)
echo $cwd_
for dir in $(find . -name "CMakeLists.txt")
do
    dir_name=$(dirname $dir)
    echo $dir_name
    cd $dir_name
        if [ -d "plots" ]; then
            echo "plots for $dirname already exists!"
        else
            echo "building $dirname"
            rm -rf build
            mkdir -p build
            cd build
                cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release > /dev/null
                ninja
            cd ..
            build/exe $SIMD_END $SIMD_CORES > /dev/null
        fi
    cd $cwd_
done
python3 generate_plots.py
python3 generate_plot.py
