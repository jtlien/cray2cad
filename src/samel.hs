
import Data.List
import Control.Monad
import System.Environment

--
--  Convert a file of columnar data to a horizontal list format
--        for example
--            California  LosAngeles
--            Wisconsin  Madison
--            Califonia  Bakersfield
--            Califoria  SanJose
--            Wisconsin  Milwaukee
--            etc
--      
--            becomes
--            California LosAngeles Bakersfield SanJose
--            Wisconsin  Madison Milwaukee
--

getLines = liftM lines . readFile

--
--  Check to see if the first words compare
--
fstEq::(String,[String])->(String,[String])->Bool
fstEq  (a,str1) (b,str2) = a == b
--
-- camparison of first word in line for sortBy
--
fstCmp::(String,[String])->(String,[String])->Ordering
fstCmp  (a,str1) (b,str2) = compare a b
--
--  convert list of vertial data lines to list of horizontal lines
--
fromVertToHoriz::[String]->[String]
fromVertToHoriz [] = []
fromVertToHoriz xs =  map toHoriz (groupBy fstEq ( sortBy (fstCmp) (map lineToPair xs)))
--
--  convert list of [(firstWord,[rest words])] -> String
--
toHoriz::[(String,[String])]->String
toHoriz xs = fst ( head xs ) ++ " " ++ concat ( map   (addSpace . unwords . snd) xs )
--
--
--
addSpace::String->String
addSpace str = ' ' : str
--
--  convert a line to a pair ( firstWord , rest of words )
--
lineToPair::String->(String,[String])
lineToPair str | (length (words str)) > 0 =   ( head $ words str, tail $ words str )
lineToPair str | length (words str) == 1 = ( head $ words str ,[])
lineToPair str  = ("",[])


main = do
    arglist <- getArgs
    list <- getLines (head arglist)
    mapM_ putStrLn $ fromVertToHoriz list
