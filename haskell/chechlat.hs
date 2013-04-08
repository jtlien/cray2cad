
import Data.Ratio
import Data.List
import System.Environment
import System.Directory
import Data.Map as M (Map, insert, empty, update, lookup, fromList, elems)


 ---          Portn  Wire
type  Incon = (String,String)
type  Outcon = (String,String)
--          Name   Inst  Inputs Outputs
type  Mod = (String,Int,[Incon],[Outcon])
--             SrcMod      Port   Dest       Port  Wire
type  DataIn = (String,Int,String,String,Int,String,String)
--
type  Modlist= [Mod]
--
--  strip off xx.3 before . and take whats after
--
instrToNum::String->Int
instrToNum s = read ( drop 1 ( snd ( break ('.'==) s) ))
--
--        xx.3 -> xx
--
getModule::String->String
getModule s = ( fst ( break ('.'==) s))
--
--
--
procDataIn::[String]->[DataIn]
procDataIn [] = []
procDataIn sl = fiveListToDataIn (take 5 sl) ++ procDataIn (drop 5 sl)
--
--    
--
fiveListToDataIn::[String]->[(String,Int,String,String,Int,String,String)]
fiveListToDataIn inl |  (length inl < 5) = error ( "expected 5 in list" ++ show inl )
fiveListToDataIn inl | otherwise = [(getModule  (head inl), instrToNum (head inl),secInL inl, getModule (thrdInL inl) , instrToNum (thrdInL inl) , frthInL inl, fifthInL inl  )]
--
--
--
secInL::[String]->String
secInL inl = head (drop 1  inl)
--
--
--
thrdInL::[String]->String
thrdInL inl = head (drop 2  inl)
--
--
--
frthInL::[String]->String
frthInL inl = head (drop 3  inl)
--
--
--
fifthInL::[String]->String
fifthInL inl = head (drop 4  inl)
--
--
--
dataInToModList::[DataIn]->[Mod]
dataInToModList dl = foldr applyModToDataIn  [] (reverse dl)
--
--
--
applyModToDataIn::DataIn->[Mod]->[Mod]
applyModToDataIn dl@(s,si,sp,d,di,dp,w) [] = [(s, si, [(sp,w)],[])] ++ [(d, di ,[],[(dp,w)])]
applyModToDataIn dl@(s,si,sp,d,di,dp,w) ml | (isInModList s si ml) && (not (isInModList d di ml))  = newInputToMl (addOutputToMl ml s si sp w)  d di dp w 
applyModToDataIn dl@(s,si,sp,d,di,dp,w) ml | (isInModList d di ml) && (not (isInModList s si ml)) = newOutputToMl (addInputToMl ml d di dp w ) s si sp w
applyModToDataIn dl@(s,si,sp,d,di,dp,w) ml | (isInModList s si ml) && (isInModList d di ml) = addOutputToMl (addInputToMl ml d di dp w ) s si sp w
applyModToDataIn dl@(s,si,sp,d,di,dp,w) ml | otherwise = error (show s ++ "." ++ show si ++ " " ++ show d ++ "." ++ show di )-- newInputToMl ( newOutputToMl ml s si sp w ) d di dp w
--
--
--
isInModList::String->Int->[Mod]->Bool
isInModList s si ml = length ( filter ( firSec s si )  ml) > 0 
--
--
--
firSec::String->Int->Mod->Bool
firSec s si (sm, smi, xs,ys) = (s==sm) && (si == smi)
--
--
--
fstof4 (a,b,c,d) = a
--
--   New Module with an input
--
newInputToMl::[Mod]->String->Int->String->String->[Mod]
newInputToMl  ml  d di dp w = ml ++ [(d,di, [(dp,w)],[])]
--
--   New Module with an output
--
newOutputToMl::[Mod]->String->Int->String->String->[Mod]
newOutputToMl  ml  s si sp w =  ml ++ [(s,si,[],[(sp,w)])]
--
--
--            List    dest  dest port   wire
addInputToMl::[Mod]->String->Int->String->String->[Mod]
addInputToMl []     d di dp w = []
addInputToMl (x:xs) d di dp w = [updateModInput x d di dp w] ++ addInputToMl xs d di dp w
--
--
--            List    dest  dest port   wire
addOutputToMl::[Mod]->String->Int->String->String->[Mod]
addOutputToMl []     s si sp w = []
addOutputToMl (x:xs) s si sp w = [updateModOutput x s si sp w] ++ addOutputToMl xs s si sp w
--
--
--
updateModInput::Mod->String->Int->String->String->Mod
updateModInput md@(mn,inum,ilist,olist) d dinum dp w  | (mn == d) && ( inum == dinum) = (mn,inum,ilist++[(dp,w)],olist)
updateModInput md@(mn,inum,ilist,olist) d dinum dp w  | otherwise = md
--
--
--
updateModOutput::Mod->String->Int->String->String->Mod
updateModOutput md@(mn,inum,ilist,olist) s sinum sp w  | (mn == s) && (inum == sinum) = (mn,inum,ilist,olist++[(sp,w)])
updateModOutput md@(mn,inum,ilist,olist) s sinum sp w  | otherwise = md
--
--
--
showMods::[Mod]->String
showMods [] = []
showMods (x:xs) = showAMod x ++ showMods xs
--
--
--
showAMod::Mod->String
showAMod (mn,inum,ilist,olist) = (showModName mn inum) ++ showInList ilist ++ showOutList (allButLast olist) ++ showOutListNC ( lastOne olist) ++ ")" ++ ['\n']
--
--
--
allButLast::[a]->[a]
allButLast [] = []
allButLast ls = reverse ( drop 1 ( reverse ls ))
--
--
--
lastOne::[a]->[a]
lastOne [] = []
lastOne ls = [head ( reverse ls)]
--
--
--
showModName::String->Int->String
showModName mn inum = mn ++ "  " ++ mn ++ "_" ++ show inum ++ " ("
--
--
--
showInList::[Incon]->String
showInList [] = []
showInList il@(x:xs) = showAnInport x ++ showInList xs
--
--
--
showInListNC::[Incon]->String
showInListNC [] = []
showInListNC il@(x:xs) = showAnInportNC x ++ showInListNC xs
--
--
--
showOutList::[Outcon]->String
showOutList [] = []
showOutList il@(x:xs) = showAnOutport x ++ showOutList xs
--
--
--
showOutListNC::[Outcon]->String
showOutListNC [] = []
showOutListNC il@(x:xs) = showAnOutportNC x ++ showOutListNC xs
--
--
--
showAnInport::Incon->String
showAnInport (sp,w) = "." ++ sp ++ "(" ++ w ++ ")" ++ "," ++ "\n"
--
--
--
showAnInportNC::Incon->String
showAnInportNC (sp,w) = "." ++ sp ++ "(" ++ w ++ ")" ++ "\n"
--
--
--
showAnOutport::Incon->String
showAnOutport (sp,w) = "." ++ sp ++ "(" ++ w ++ ")" ++ "," ++ "\n"
--
--
--
showAnOutportNC::Incon->String
showAnOutportNC (sp,w) = "." ++ sp ++ "(" ++ w ++ ")" ++ "\n"
--
--
--
doMods::[String]->String
doMods sl = showMods ( dataInToModList ( procDataIn ( sl ) ))


main = do args <- getArgs
          contents <- readFile (head args)
          let ws = concatMap words ( lines contents)
          putStrLn (doMods ws)
