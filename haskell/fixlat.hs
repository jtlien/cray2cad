import Data.Ratio
import Data.List
import System.Environment

--
-- take the line xxx = aaa + ccc etc
--     return the left hand side
--        ie xxx
--
getLhs::String->String
getLhs str | elem '=' str =   filter (neSpace) (fst (takeUntilA '=' str))
getLhs str  | otherwise = ""
--
--
--
neSpace::Char->Bool
neSpace ch | ch == ' ' = False
neSpace ch | otherwise = True

--
--  return the right hand side
--
getRhs::String->String
getRhs str = snd (takeUntilA '=' str)
--
--
--
takeUntilA :: Char  -> String -> (String,String)
takeUntilA c xs =
    (takeWhile (/= c) xs, dropWhile (/= c) xs)
--
--
--
cartProd xs ys = [(x,y) | x <- xs, y <- ys]
--
--
--
pairEq::(String,String)->Bool
pairEq (x,y) | x == y = True
pairEq (x,y) | otherwise = False
--
--
fstThree::(Int,String,String)->Int
fstThree (a,b,c) = a
--
--
--
fixFun::[(Int,String,String)]->String->([(Int,String,String)],String)
fixFun [] line =  ( [(1, getLhs line, getRhs line)], line)
fixFun prs line  =  ( prs ++ [ ( ( fstThree ( head ( reverse prs))) + 1, getLhs line,getRhs line)], line)
--
--
--
lenGt2::String->Bool
lenGt2 str | length str > 2 = True
lenGt2 str | otherwise = False
--
--
--
fixNeg::String->String
fixNeg [] = []
fixNeg str | head str == '~' = tail str
fixNeg str | otherwise = str
--
--   check each line using a list of 
--     of Lhs terms accumulated, to see if any rhs trigraph is already
--                       defined, if so this is an error
--
doFix::[String]->([(Int,String,String)],[String])
doFix lns = mapAccumL (fixFun) [] lns 
--
--
--
addEol::String->String
addEol str = str ++ "\n"
--
--
--
fmtLine::((Int,String,String),String)->String
fmtLine ((n,b,c),d) | eqFnd d = "V[" ++ show n ++ "]" ++ c ++ " \n"
fmtLine ((n,b,c),d) | elem '}' d = "\n"
fmtLine ((n,b,c),d) | otherwise = d ++ "\n"
--
--
--
fmtLine2::(Int,String,String)->String
fmtLine2 (n,b,c) | b == "" = "\n"
fmtLine2 (n,b,c) | otherwise = b ++ " = " ++ "V[" ++ show n ++ "];\n"
--
--
--
eqFnd::String->Bool
eqFnd str = elem '=' str
--
--
--
main = do args <- getArgs
          contents <- readFile (head args)
          let fres = doFix (lines contents)
          let res = fst (doFix  (lines contents))
          putStrLn ( concat ( map (fmtLine) (zip  (fst fres ) (snd fres))))
          putStrLn ( concat ( map (fmtLine2) res ))
          putStrLn ( " } \n" )