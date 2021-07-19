set(subdirs 
    find
    find_if
    find_if_not
    find_first_of
    find_end
)

foreach(subdir ${subdirs})
    message("Adding subdir ${subdir}")
    add_subdirectory(${subdir})
endforeach(subdir)
