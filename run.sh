for dir in $(ls -d */)
do
    echo $dir
    cd $dir
    bash generate_plots.sh
    cd ..
done
python3 merge_plots.py
