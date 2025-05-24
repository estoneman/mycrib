DROP TABLE IF EXISTS movies;

CREATE TABLE movies (
    id INTEGER NOT NULL PRIMARY KEY,                                                                        -- internal id
    title TEXT UNIQUE NOT NULL CHECK (LENGTH(genre) < 128),                                                 -- title of movie
    genre TEXT NOT NULL CHECK (LENGTH(genre) < 128),                                                        -- movie genre
    year INTEGER NOT NULL CHECK (year >= 1888),                                                             -- release year, 1888 = oldest film relesed
    `length` INTEGER NOT NULL CHECK (`length` > 0 AND `length` <= 65535),                                   -- length of the movie in MINUTES
    poster_url TEXT CHECK (LENGTH(poster_url) < 512),                                                       -- for displaying in app
    rating_family REAL DEFAULT 'N/A' CHECK (rating_family > 0.0 AND rating_family < 10.0),                  -- family rating
    `cast` TEXT NOT NULL CHECK (LENGTH(`cast`) < 512),                                                      -- delimited list of cast members (':' delimited)
    director TEXT NOT NULL CHECK (LENGTH(director) < 128),                                                  -- director(s) of movies (':' delimited)
    rating_imdb REAL DEFAULT 0.0 CHECK (rating_imdb > 0.0 AND rating_imdb < 10.0),                          -- imdb rating
    created_at TEXT DEFAULT CURRENT_TIMESTAMP,                                                              -- date record was inserted
    updated_at TEXT DEFAULT CURRENT_TIMESTAMP                                                               -- date record was updated
);

INSERT INTO movies (id, title, genre, year, length, poster_url, rating_family, cast, director, rating_imdb)
VALUES (1, 'The Matrix', 'Science Fiction', 1999, 136, 'https://example.com/matrix.jpg', 7.5, 'Keanu Reeves:Laurence Fishburne:Carrie-Anne Moss', 'Lana Wachowski:Lilly Wachowski', 8.7);

INSERT INTO movies (id, title, genre, year, length, poster_url, rating_family, cast, director, rating_imdb)
VALUES (2, 'The Young Teacher', 'Drama', 1976, 95, 'https://example.com/young_teacher.jpg', 6.1, 'Actor A:Actor B', 'Director A', 6.9);

INSERT INTO movies (id, title, genre, year, length, poster_url, rating_family, cast, director, rating_imdb)
VALUES (3, 'Inception', 'Thriller', 2010, 148, 'https://example.com/inception.jpg', 8.2, 'Leonardo DiCaprio:Joseph Gordon-Levitt:Elliot Page', 'Christopher Nolan', 8.8);

INSERT INTO movies (id, title, genre, year, length, poster_url, rating_family, cast, director, rating_imdb)
VALUES (4, 'Spirited Away', 'Animation', 2001, 125, 'https://example.com/spirited.jpg', 7.9, 'Rumi Hiiragi:Miyu Irino', 'Hayao Miyazaki', 8.6);

INSERT INTO movies (id, title, genre, year, length, poster_url, rating_family, cast, director, rating_imdb)
VALUES (5, 'Parasite', 'Thriller', 2019, 132, 'https://example.com/parasite.jpg', 7.0, 'Song Kang-ho:Lee Sun-kyun:Cho Yeo-jeong', 'Bong Joon-ho', 8.6);

INSERT INTO movies (id, title, genre, year, length, poster_url, rating_family, cast, director, rating_imdb)
VALUES (6, 'Amadeus', 'Biography', 1984, 160, 'https://example.com/amadeus.jpg', 8.0, 'F. Murray Abraham:Tom Hulce', 'Milos Forman', 8.4);

INSERT INTO movies (id, title, genre, year, length, poster_url, rating_family, cast, director, rating_imdb)
VALUES (7, 'The Godfather', 'Crime', 1972, 175, 'https://example.com/godfather.jpg', 9.0, 'Marlon Brando:Al Pacino:James Caan', 'Francis Ford Coppola', 9.2);

INSERT INTO movies (id, title, genre, year, length, poster_url, rating_family, cast, director, rating_imdb)
VALUES (8, 'Arrival', 'Science Fiction', 2016, 116, 'https://example.com/arrival.jpg', 7.8, 'Amy Adams:Jeremy Renner', 'Denis Villeneuve', 7.9);

INSERT INTO movies (id, title, genre, year, length, poster_url, rating_family, cast, director, rating_imdb)
VALUES (9, 'Moonlight', 'Drama', 2016, 111, 'https://example.com/moonlight.jpg', 7.5, 'Trevante Rhodes:Mahershala Ali', 'Barry Jenkins', 7.4);

INSERT INTO movies (id, title, genre, year, length, poster_url, rating_family, cast, director, rating_imdb)
VALUES (10, 'Get Out', 'Horror', 2017, 104, 'https://example.com/getout.jpg', 7.0, 'Daniel Kaluuya:Allison Williams', 'Jordan Peele', 7.7);
