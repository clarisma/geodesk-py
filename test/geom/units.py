# Factor to convert meters to the given unit:
# value_in_unit = value_in_meters * factor

LENGTH_UNITS = [
    ("meters",      1.0),
    ("m",           1.0),
    ("kilometers",  0.001),
    ("km",          0.001),
    ("feet",        3.280839895013123),
    ("ft",          3.280839895013123),
    ("yards",       1.0936132983377078),
    ("yd",          1.0936132983377078),
    ("miles",       0.0006213711922373339),
    ("mi",          0.0006213711922373339),
]

# Factor to convert square meters to the given unit:
# value_in_unit = value_in_square_meters * factor

AREA_UNITS = [
    ("meters",              1.0),
    ("m",                   1.0),
    ("kilometers",          1e-6),
    ("km",                  1e-6),
    ("feet",                10.763910416709722),
    ("ft",                  10.763910416709722),
    ("yards",               1.1959900463010802),
    ("yd",                  1.1959900463010802),
    ("miles",               3.8610215854244586e-7),
    ("mi",                  3.8610215854244586e-7),

    ("square_meters",       1.0),
    ("m2",                  1.0),
    ("square_kilometers",   1e-6),
    ("km2",                 1e-6),
    ("square_feet",         10.763910416709722),
    ("ft2",                 10.763910416709722),
    ("square_yards",        1.1959900463010802),
    ("yd2",                 1.1959900463010802),
    ("square_miles",        3.8610215854244586e-7),
    ("mi2",                 3.8610215854244586e-7),

    ("hectares",            1e-4),
    ("hc",                  1e-4),
    ("acres",               0.00024710538146716534),
    ("ac",                  0.00024710538146716534),
]