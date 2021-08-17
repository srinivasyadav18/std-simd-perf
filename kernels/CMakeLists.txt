set(subdirs
    mandel
)

foreach(subdir ${subdirs})
    message("Adding subdir ${subdir}")
    add_subdirectory(${subdir})
endforeach(subdir)
