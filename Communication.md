## Client -> Server:

- [x] 01\[nickname] - assign a nickname
- [x] 02\[name,password,level,rounds,time] - create a new room with the given name
- [x] 03\[name,password] - join a room with the given name
- [x] 06\[letter] - attempt to guess a letter
- [x] 07\[] - restart the game
- [x] 08\[room] - request the current game state
- [x] 09\[room] - leave the room
- [x] 70\[] - request the list of rooms
- [x] 71\[] - request the list of player nicknames in a given room
- [x] 73\[] - request to start the game in the player's room
- [x] 80\[] - request a new round (time has expired)
- [x] 82\[] - signal readiness to play

## Server -> Client:
- [x] 01\[bool] - confirmation of nickname assignment
- [x] 02\[bool] - confirmation of room creation
- [x] 03\[bool] - confirmation of joining a room
- [x] 06\[bool,letter,<lives>] - response to a letter guess attempt (lives if bool = 0)
- [x] 11\[] - update remaining round time
- [x] 12\[] - round time expired
- [x] 69\[hello!] - connection confirmation
- [x] 70\[name1,..,name99] - send the list of room names
- [x] 71\[name1,..,name99] - send the list of player names in a given room
- [x] 72\[bool] - permission to start the game (0 if players leave the waiting room before the game starts and there are fewer than 2)
- [x] 73\[encrypted_word,time,rounds,your_nickname,opponents_list] - signal to start a new round (first time means transitioning to the game page)
- [x] 74\[nickname] - the client with the given nickname left the game mid-session
- [x] 75\[word] - update game word information
- [x] 76\[nickname:lives] - update opponent’s hangman status
- [x] 77\[nickname:points] - update ranking
- [x] 78\[nickname:points,nickname:points;word1,word2] - end of the game
- [x] 79\[encrypted_word,round,current_round,max_rounds] - new round
- [x] 81\[nickname] - a player left the room during the game
- [x] 82\[bool] - response to readiness to play
- [x] 83\[] - player was kicked from the game because they were the only one in the room
 
