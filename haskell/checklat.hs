
import Data.Ratio
import Data.List
import System.Environment

--
-- take the line xxx = aaa + ccc etc
--     return the left hand side
--        ie xxx
--
getLhs::String->String
getLhs str = filter (neSpace) (fst (takeUntilA '=' str))
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
--
chkFun::[String]->String->([String],String)
chkFun defl line | hasOneInList defl ( splitUpRHS (getRhs line) ) = ( defl ++ [getLhs line], getLhs line)
chkFun defl line | otherwise = ( defl ++ [getLhs line], "")

--
--  check to see if any of the terms on the right hand side are already
--                defined
--
hasOneInList::[String]->[String]->Bool
hasOneInList defl terml | length (filter (pairEq) ( cartProd defl terml)) > 0 = True
hasOneInList defl terml | otherwise = False

--
--     Get the trigraphs
--
splitUpRHS::String->[String]
splitUpRHS str = map (fixNeg) (filter (lenGt2) (words str))
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
doSearch::[String]->([String],[String])
doSearch lns = mapAccumL (chkFun) [] lns 
--
--
--
addEol::String->String
addEol str = str ++ "\n"

--
--
--
main = do args <- getArgs
          contents <- readFile (head args)
          let errs = snd( doSearch  (lines contents))
          putStrLn ( concat ( map (addEol) (filter (lenGt2) errs)))
