import os

for root, subdirs, files in os.walk(os.getcwd()):
    for file in files:
        if file.endswith(".intohylo") and not file.startswith("._"):
            with open(root + "/"+file, "r") as f:
                text = f.readlines()[1]
            text = text.replace("~ ", "~")
            text = text.replace("[r1", "[")
            text = text.replace("<r1", "<")
            with open(root+file.replace(".intohylo", ".hf"), "w") as f:
                f.write(text)
            text = text.replace("true", "$true")
            text = text.replace("false", "$false")
            with open(root+file.replace(".intohylo", ".cf"), "w") as f:
                f.write(text)
