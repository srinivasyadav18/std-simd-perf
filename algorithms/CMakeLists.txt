set(subdirs
    adjacent_difference
    adjacent_find
    all_any_none 
    copy
    count
    fill
    find
    for_each
    generate
    transform
)

foreach(subdir ${subdirs})
    message("Adding subdir ${subdir}")
    add_subdirectory(${subdir})
endforeach(subdir)
