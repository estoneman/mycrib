DROP TABLE IF EXISTS movies;

CREATE TABLE movies (
    id INTEGER NOT NULL PRIMARY KEY,            -- internal id
    title TEXT NOT NULL,                        -- title of movie
    year INTEGER NOT NULL,                      -- release year
    director TEXT NOT NULL,                     -- director(s) of movies (':' delimited)
    `cast` TEXT NOT NULL,                       -- delimited list of cast members (':' delimited)
    genre TEXT NOT NULL,                        -- movie genre
    duration INTEGER NOT NULL,                  -- length of the movie in MINUTES
    rating_family REAL DEFAULT 'N/A',           -- family rating
    rating_imdb REAL DEFAULT 'N/A',             -- imdb rating
    poster_url TEXT,                            -- for displaying in app
    created_at TEXT DEFAULT CURRENT_TIMESTAMP,  -- date record was inserted
    updated_at TEXT DEFAULT CURRENT_TIMESTAMP   -- date record was updated
);

INSERT INTO movies (title, year, director, `cast`, genre, duration)
VALUES
('The Shawshank Redemption', 1994, 'Frank Darabont', 'Tim Robbins:Morgan Freeman:Bob Gunton', 'Drama', 142);

INSERT INTO movies (title, year, director, `cast`, genre, duration, rating_family, rating_imdb)
VALUES
('The Lion King', 1994, 'Roger Allers:Rob Minkoff', 'Matthew Broderick:James Earl Jones:Jeremy Irons', 'Animation', 88, 8.5, 8.5);

INSERT INTO movies (title, year, director, `cast`, genre, duration, rating_imdb, poster_url)
VALUES
('Inception', 2010, 'Christopher Nolan', 'Leonardo DiCaprio:Joseph Gordon-Levitt:Elliot Page', 'Science Fiction', 148, 8.8, 'https://example.com/inception.jpg');

INSERT INTO movies (title, year, director, `cast`, genre, duration, rating_family)
VALUES
('Finding Nemo', 2003, 'Andrew Stanton', 'Albert Brooks:Ellen DeGeneres:Alexander Gould', 'Animation', 100, 8.1);

INSERT INTO movies (title, year, director, `cast`, genre, duration)
VALUES
('The Godfather', 1972, 'Francis Ford Coppola', 'Marlon Brando:Al Pacino:James Caan', 'Crime', 175);
