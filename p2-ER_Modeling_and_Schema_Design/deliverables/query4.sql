SELECT A.ItemID FROM Item AS A, 
(SELECT MAX(CAST(Currently AS REAL)) AS maxVal FROM Item) AS B
WHERE CAST(A.Currently AS REAL) = B.maxVal;