CREATE TABLE users (id INT PRIMARY KEY, name TEXT);
INSERT INTO users VALUES (1, 'Akshay');
INSERT INTO users VALUES (2, 'IIT Hyderabad');
SELECT * FROM users;
SELECT * FROM users WHERE id = 1;
UPDATE users SET name = 'Akshay Bagde' WHERE id = 1;
DELETE FROM users WHERE id = 2;
SHOW TABLES;
EXIT;
