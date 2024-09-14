# replace_icon_text.py - Replace icon code in h and cpp files
# parse the icon xml metadata file

import glob

# gen translation dict
translation_dict = {
    "icons/add.png":"icons/add-diagram.svg", # edited
    "icons/adjust.png":"icons/adjust.png",
    "icons/adjust-2.png":"icons/tune.svg", # edited
    "icons/arrow.png":"icons/arrow.png",
    "icons/clock.png":"icons/history.svg", # edited
    "icons/copy.png":"icons/copy.svg", # edited
    "icons/compare.png":"icons/diff.svg", # edited
    "icons/data.png":"icons/data.png",
    "icons/electronic-clock.png":"icons/electronic-clock.png",
    "icons/edit.png":"icons/edit.svg", # edited
    "icons/list.png":"icons/table.svg", # edited
    "icons/info.png":"icons/info.svg", # edited
    "icons/timetable.png":"icons/timetable.png",
    "icons/search.png":"icons/search.svg", # edited
    "icons/ruler.png":"icons/ruler.svg", # edited
    "icons/polyline.png":"icons/polyline.png",
    "icons/text.png":"icons/text.png",
    "icons/identify.png":"icons/identify.png",
    "icons/exchange.png":"icons/switch.svg", # edited
    "icons/rail.png":"icons/add-track.svg", # edited
    "icons/database.png":"icons/application-sql.svg", # edited
    "icons/excel.png":"icons/excel.png",
    "icons/joint.png":"icons/joint.png",
    "icons/forbid.png":"icons/forbid.png",
    "icons/exchange1.png":"icons/exchange1.png",
    "icons/new-file.png":"icons/new-file.png",
    "icons/save1.png":"icons/save1.png",
    "icons/saveas.png":"icons/saveas.png",
    "icons/open.png":"icons/open.png",
    "icons/ETRC-dynamic.png":"icons/ETRC-dynamic.png",
    "icons/pdf.png":"icons/application-pdf.svg", # edited
    "icons/png.png":"icons/png.png",
    "icons/refresh.png":"icons/refresh.png",
    "icons/brush.png":"icons/brush.png",
    "icons/close.png":"icons/close.png",
    "icons/icon.ico":"icons/icon.ico",
    "icons/help.png":"icons/help.png",
    "icons/menu.png":"icons/menu.png",
    "icons/h_expand.png":"icons/h_expand.png",
    "icons/h_shrink.png":"icons/h_shrink.png",
    "icons/v_expand.png":"icons/v_expand.png",
    "icons/v_shrink.png":"icons/v_shrink.png",
    "icons/add_train.png":"icons/add_train.png",
    "icons/filter.png":"icons/filter.png",
    "icons/config.png":"icons/config.png",
    "icons/settings.png":"icons/settings.png",
    "icons/zoom-in.png":"icons/zoom-in.png",
    "icons/zoom-out.png":"icons/zoom-out.png",
    "icons/counter.png":"icons/counter.png",
    "icons/train.png":"icons/train.svg", # edited
    "icons/ruler_pen.png":"icons/toll.svg", # edited
    "icons/Graph-add.png":"icons/Graph-add.png",
    "icons/diagram.png":"icons/diagram.png",
    "icons/redo.png":"icons/redo.png",
    "icons/undo.png":"icons/undo.png",
    "icons/tick.png":"icons/tick.png",
    "icons/trainline.png":"icons/trainline.png",
    "icons/line-manage.png":"icons/line-manage.png",
    "icons/weaken.png":"icons/weaken.png",
    "icons/routing-diagram.png":"icons/routing-diagram.png",
    "icons/mobile-icon.png":"icons/mobile-icon.png",
    "icons/customize.svg":"icons/customize.svg",
    "icons/icon-transparent.png":"icons/icon-transparent.png",
}

# copy the original file
with open("rsc/resource.bak", "r") as f:
    with open("rsc/resource.qrc", "w") as f1:
        f1.write(f.read())

with open("rsc/resource.qrc", "r") as f:
    # replace
    lines = []
    for line in f:
        for key, item in translation_dict.items():
            if key in line:
                lines.append(line.replace(key, item))
                break
        else:
            lines.append(line)

    with open("rsc/resource.qrc", "w") as f1:
        f1.writelines(lines)

# replace everything in cpp and h files
src_files = glob.glob("src/**/*.cpp", recursive=True) + glob.glob("src/**/*.h", recursive=True)
print(len(src_files))
for src_file in src_files:
    with open(src_file, "r") as f:
        existing_lines = f.readlines()
        lines = []
        for line in existing_lines:
            for key, item in translation_dict.items():
                if key.split("/")[1] in line:
                    lines.append(line.replace(key.split("/")[1], item.split("/")[1]))
                    break
            else:
                lines.append(line)

    # don't write if the lines are identical
    if lines == existing_lines:
        continue

    with open(src_file, "w") as f1:
        f1.writelines(lines)
