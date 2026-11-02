# Geography of the Dominion for worldmap.py, derived from the zone files.
# Positions are normalised (x right, y down).  Relative placement follows the
# directions of the exits between zones (the exits between zones), with themes taken from
# room names, sector types and the mobs each zone loads.

# kind: capital city town keep tower temple cave lair camp
ZONES = {
    30:  ("Elrhinn Township",        0.470, 0.500, "capital"),
    200: ("Kobold Catacombs",        0.478, 0.585, "cave"),
    39:  ("River Island of Peringuard", 0.452, 0.800, "keep"),
    50:  ("Cair-Irynthar",           0.525, 0.790, "city"),
    47:  ("Battlelands",             0.560, 0.725, "camp"),
    56:  ("Chree'zen",               0.530, 0.660, "town"),
    255: ("Druid's Glade",           0.408, 0.455, "temple"),
    18:  ("Darkweir Hollows",        0.385, 0.360, "camp"),
    326: ("Pelarinne Forest",        0.355, 0.535, "camp"),
    110: ("Three of Swords",         0.330, 0.640, "town"),
    66:  ("Dragon Spyre",            0.268, 0.725, "lair"),
    40:  ("Mines of the Hills",      0.200, 0.745, "cave"),
    25:  ("Dark Tower",              0.295, 0.800, "tower"),
    250: ("Western Mountain",        0.195, 0.600, "keep"),
    106: ("Cliffs of Rhaylinne",     0.240, 0.495, "camp"),
    117: ("Rhaylinne Tower",         0.200, 0.455, "tower"),
    410: ("Desert of Desolation",    0.285, 0.385, "town"),
    11:  ("Mytherandil",             0.195, 0.285, "city"),
    53:  ("The Arena",               0.110, 0.300, "camp"),
    328: ("Kehrn Stronghold",        0.215, 0.165, "keep"),
    52:  ("Caverns of the Underdark", 0.300, 0.190, "cave"),
    360: ("Castle Black",            0.375, 0.125, "keep"),
    21:  ("Hills of Nord",           0.455, 0.170, "camp"),
    352: ("Thallin Plains",          0.470, 0.265, "camp"),
    64:  ("The Monk's Monastery",    0.468, 0.375, "temple"),
    353: ("Spiddel Forest",          0.535, 0.275, "camp"),
    350: ("Gnishmire",               0.585, 0.230, "city"),
    55:  ("Elrhinn Hills",           0.570, 0.370, "camp"),
    75:  ("Blackwood Forest",        0.540, 0.450, "camp"),
    190: ("The Elven Woods",         0.585, 0.540, "town"),
    320: ("Serdim-ir-Lesk",          0.660, 0.415, "town"),
    80:  ("Bryn Mawr",               0.700, 0.275, "keep"),
    81:  ("The Wastelands",          0.755, 0.335, "camp"),
    100: ("Charing Cross",           0.765, 0.450, "city"),
    260: ("Thangorodrim",            0.885, 0.375, "lair"),
    383: ("Castle of Llyndyr",       0.820, 0.215, "keep"),
    45:  ("Tomb of Haqim-Taar",      0.815, 0.590, "cave"),
    280: ("The Eastern Plains",      0.890, 0.545, "camp"),
    61:  ("Haon-Dor",                0.640, 0.610, "camp"),
    63:  ("Arachnos",                0.670, 0.520, "lair"),
    37:  ("High Tower of Magic",     0.735, 0.600, "tower"),
    104: ("Ancalador",               0.712, 0.705, "cave"),
    105: ("Vice Island",             0.690, 0.880, "town"),
    330: ("The Labyrinth",           0.765, 0.795, "cave"),
    27:  ("Ganin's World",           0.500, 0.625, "cave"),
    420: ("The Weeping Wood",        0.615, 0.470, "cave"),
    430: ("Valley of Fate",          0.305, 0.470, "temple"),
    58:  ("Isle of Karras",          0.585, 0.900, "keep"),
}

# Notable foes worth a mark on the map: zone -> (name, level)
FOES = {
    52:  ("the Dracolich", 115),
    39:  ("King Peringuard", 50),
    64:  ("Grand Master of Monks", 50),
    50:  ("Lord of the Black Watch", 48),
    66:  ("the Eternity Dragon", 42),
    260: ("Melkor Baughlith", 42),
    383: ("the Emperor", 42),
    190: ("the Dark Figure", 42),
    250: ("Jirdidan the Ancient One", 41),
    18:  ("the Treant", 40),
    105: ("Grand Dragon of Terror", 40),
    37:  ("the Grand Mistress", 40),
    328: ("Kehrn the Troll King", 37),
    110: ("Aglandiir the Dragon Prince", 35),
    360: ("the Ice Monster", 33),
    63:  ("Arachnos & Yevaud", 26),
    104: ("the Crypt Keeper", 25),
    106: ("the Iron Golem", 25),
    61:  ("the Green Dragon", 20),
    58:  ("Krommyon of the Kinthalas", 30),
    420: ("Morvane the Dark Twin", 28),
    27:  ("King Vrask", 10),
}

# Land: (x, y, rx, ry) ellipses, unioned and roughened with noise.
LAND = [
    (0.47, 0.50, 0.17, 0.30), (0.31, 0.45, 0.15, 0.30), (0.17, 0.36, 0.11, 0.21),
    (0.21, 0.66, 0.10, 0.15), (0.28, 0.79, 0.055, 0.07), (0.60, 0.40, 0.10, 0.20),
    (0.67, 0.61, 0.07, 0.12), (0.63, 0.67, 0.05, 0.07), (0.625, 0.62, 0.04, 0.06), (0.715, 0.71, 0.035, 0.06), (0.53, 0.73, 0.065, 0.10),
    (0.44, 0.18, 0.13, 0.10), (0.26, 0.15, 0.11, 0.09), (0.72, 0.30, 0.06, 0.11),
    (0.77, 0.43, 0.05, 0.035), (0.875, 0.39, 0.065, 0.12), (0.86, 0.56, 0.07, 0.08),
    (0.82, 0.21, 0.055, 0.075), (0.30, 0.21, 0.045, 0.045), (0.80, 0.60, 0.04, 0.05), (0.07, 0.30, 0.04, 0.08),
    # islands
    (0.69, 0.875, 0.036, 0.045), (0.765, 0.795, 0.026, 0.04), (0.075, 0.58, 0.018, 0.03),
    (0.585, 0.90, 0.032, 0.045), (0.955, 0.62, 0.015, 0.03), (0.39, 0.93, 0.02, 0.02),
    (0.06, 0.80, 0.025, 0.03), (0.96, 0.26, 0.015, 0.025),
    (0.14, 0.88, 0.018, 0.025),
]
# Water cut back out of the land.
SEA = [
    (0.635, 0.155, 0.032, 0.075), (0.375, 0.875, 0.06, 0.065), (0.600, 0.845, 0.038, 0.06),
    (0.80, 0.31, 0.022, 0.055), (0.795, 0.50, 0.022, 0.035), (0.13, 0.50, 0.030, 0.035),
]
LAKES = [  # (x, y, rx, ry)
    (0.485, 0.248, 0.012, 0.016),   # Thallin lake
    (0.668, 0.432, 0.016, 0.022),   # Serdim-ir-Lesk
    (0.232, 0.205, 0.008, 0.012),   # Kehrn pool
    (0.243, 0.505, 0.010, 0.012),   # Rhaylinne basin tarn
]
RIVERS = [  # list of points, source first; width at mouth
    ([(0.485, 0.262), (0.478, 0.33), (0.470, 0.42), (0.468, 0.50), (0.470, 0.60),
      (0.462, 0.70), (0.452, 0.790), (0.446, 0.845)], 1.0),               # Widmans
    ([(0.452, 0.790), (0.462, 0.815), (0.467, 0.85)], 0.7),                 # delta arm
    ([(0.215, 0.56), (0.185, 0.62), (0.150, 0.655), (0.105, 0.69)], 0.85),  # Uller
    ([(0.668, 0.452), (0.655, 0.52), (0.645, 0.60), (0.630, 0.68), (0.615, 0.75)], 0.8),  # dark river
    ([(0.700, 0.300), (0.672, 0.255), (0.650, 0.215)], 0.55),               # Downhill river
    ([(0.232, 0.218), (0.215, 0.26), (0.225, 0.32), (0.245, 0.35)], 0.45),  # Kehrn stream
    ([(0.870, 0.45), (0.875, 0.52), (0.900, 0.60), (0.925, 0.66)], 0.6),    # eastern river
]

# Biome anchors: (biome, x, y, rx, ry, strength)
BIOMES = [
    ("desert", 0.12, 0.31, 0.09, 0.13, 1.0), ("desert", 0.28, 0.39, 0.07, 0.07, 1.0),
    ("desert", 0.20, 0.38, 0.05, 0.06, 0.8),
    ("ash", 0.885, 0.38, 0.075, 0.11, 1.2), ("waste", 0.76, 0.33, 0.045, 0.06, 1.0),
    ("waste", 0.79, 0.20, 0.03, 0.04, 0.6),
    ("swamp", 0.665, 0.42, 0.04, 0.05, 1.0), ("swamp", 0.545, 0.455, 0.022, 0.03, 0.8),
    ("forest", 0.385, 0.36, 0.045, 0.055, 1.0), ("forest", 0.41, 0.455, 0.028, 0.03, 1.0),
    ("forest", 0.355, 0.535, 0.05, 0.055, 1.0), ("forest", 0.585, 0.545, 0.042, 0.05, 1.0),
    ("forest", 0.535, 0.28, 0.035, 0.04, 1.0), ("darkforest", 0.64, 0.615, 0.045, 0.065, 1.2),
    ("darkforest", 0.545, 0.45, 0.025, 0.03, 1.0), ("darkforest", 0.67, 0.52, 0.02, 0.025, 1.0),
    ("forest", 0.735, 0.595, 0.02, 0.025, 1.0), ("forest", 0.53, 0.66, 0.028, 0.03, 1.0),
    ("forest", 0.765, 0.795, 0.02, 0.03, 1.2), ("forest", 0.268, 0.69, 0.02, 0.02, 0.8),
    ("forest", 0.69, 0.875, 0.015, 0.02, 0.6),
    ("darkforest", 0.615, 0.470, 0.022, 0.028, 1.2), ("forest", 0.305, 0.470, 0.025, 0.03, 1.0),
    ("darkforest", 0.585, 0.900, 0.025, 0.035, 1.2),
    ("hills", 0.57, 0.37, 0.03, 0.035, 1.0), ("hills", 0.815, 0.59, 0.035, 0.04, 1.0),
    ("hills", 0.455, 0.17, 0.04, 0.04, 0.9), ("hills", 0.33, 0.645, 0.03, 0.035, 1.0),
    ("hills", 0.20, 0.745, 0.035, 0.035, 1.0), ("hills", 0.89, 0.545, 0.03, 0.03, 0.6), ("hills", 0.235, 0.50, 0.03, 0.04, 0.9),
    ("plains", 0.47, 0.265, 0.04, 0.04, 1.0), ("plains", 0.89, 0.55, 0.05, 0.05, 0.9),
    ("scar", 0.56, 0.725, 0.022, 0.025, 1.0),
]
# Mountain ranges for peak glyphs: (x, y, rx, ry, density, size, snowy)
MOUNTAINS = [
    (0.225, 0.150, 0.075, 0.045, 1.0, 1.25, True),   # Kehrn Mountains
    (0.305, 0.185, 0.035, 0.030, 1.0, 1.0, True),
    (0.190, 0.585, 0.030, 0.075, 1.0, 1.25, True),   # Western Mountains
    (0.262, 0.745, 0.030, 0.035, 1.0, 1.15, True),   # Dragon Spyre
    (0.700, 0.265, 0.040, 0.050, 1.0, 1.1, True),    # Bryn Mawr range
    (0.755, 0.330, 0.030, 0.035, 0.7, 0.9, False),   # Wastelands
    (0.712, 0.700, 0.022, 0.030, 0.9, 0.9, False),   # Ancalador
    (0.885, 0.345, 0.045, 0.055, 1.0, 1.1, False),   # Thangorodrim
    (0.335, 0.635, 0.020, 0.020, 0.5, 0.7, False),   # Three of Swords ridge
    (0.820, 0.205, 0.030, 0.035, 0.7, 0.9, True),    # Llyndyr heights
]
CLIFF_RING = (0.240, 0.500, 0.030, 0.045)  # Rhaylinne basin

REGION_LABELS = [  # (text, x, y, size, kind, arc)  kind: land/sea
    ("THE DOMINION", 0.405, 0.712, 0.9, "land", 0.0),
    ("Kehrn Mountains", 0.240, 0.092, 0.55, "land", 0.0),
    ("Desert of Desolation", 0.270, 0.345, 0.50, "land", 0.0),
    ("Desert of Demise", 0.105, 0.355, 0.45, "land", 0.0),
    ("Haon-Dor, the Dark Forest", 0.625, 0.680, 0.38, "land", 0.0),
    ("The Ashen Plains", 0.900, 0.440, 0.45, "land", 0.0),
    ("The Northern Sea", 0.690, 0.140, 0.55, "sea", 0.0),
    ("The Southern Sea", 0.47, 0.955, 0.75, "sea", 0.0),
    ("Sea of Tears", 0.615, 0.845, 0.40, "sea", 0.0),
    ("The Western Sea", 0.035, 0.46, 0.60, "sea", -90),
    ("The Eastern Sea", 0.965, 0.47, 0.60, "sea", 90),
    ("The Frozen North", 0.100, 0.070, 0.55, "land", 0.0),
]
