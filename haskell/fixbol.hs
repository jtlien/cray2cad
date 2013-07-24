
import Data.List as L
import Data.List.Split
import Data.Char
import Control.Monad
import System.Environment

--
-- 
getLines = liftM L.lines . readFile

--
-- 
--
fixBool::[String]->[String]
fixBool [] = []
fixBool xs = map fixB xs
--
--  If lower case string, then return upper case
--

allLower::String->Bool
allLower [] = True
allLower (x:xs) = (isLower x) && (allLower xs)
--
--
--
negOp::String->String
negOp [] = []
negOp xs | (head xs == '~') && (allLower ( drop 1 xs)) = drop 1 ( map ( toUpper) xs )
negOp xs | (allLower xs) = "~" ++ (map (toUpper) xs ) 
negOp xs | otherwise = xs

--
--  Take a line and fix it
--
fixB::String->String
fixB [] = []
fixB xs = intercalate " " ( map (negOp) (words xs )) 
--
--
addSpace::String->String
addSpace str = ' ' : str


main = do
    arglist <- getArgs
    list <- getLines (L.head arglist)
    mapM_ putStrLn $ fixBool list
