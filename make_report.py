import argparse

from assisivibe.common import util

parser = argparse.ArgumentParser (
    description = "create a HTML file with a report of video analysis",
    argument_default = None
)
parser.add_argument (
    '--same-colour-threshold', '-c',
    type = int,
    required = True,
    help = "threshold used to compare two pixel intensity values"
)
parser.add_argument (
    '--delta-frame', '-d',
    type = int,
    required = True,
    help = "how many frames apart were used to compute bee speed"
)
args = parser.parse_args ()

rows = util.load_csv ("data-plots.csv", True)

filename = "report_DF={delta_frame}_SCT={same_colour_threshold}.html".format (
    same_colour_threshold = args.same_colour_threshold,
    delta_frame = args.delta_frame
)
fd = open (filename, "w")
fd.write ("""<html>
  <head>
    <meta charset="UTF-8">
    <title>Vibration Pattern Analysis Report</title>
  </head>
<body>""")

for a_row in rows:
    fd.write ("""
<h1>{title}</h1>
<p>
<img src="{path}/number-bees_SCT={same_colour_threshold}.png">
<img src="{path}/bee-speed_DF={delta_frame}_SCT={same_colour_threshold}.png">""".format (
        title = a_row [1],
        path = a_row [0],
        same_colour_threshold = args.same_colour_threshold,
        delta_frame = args.delta_frame
    ))
fd.write ("""
</body>
</html>""")
fd.close ()
