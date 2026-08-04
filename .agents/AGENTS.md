# Project Rules

## Granular Git Commits & Automatic Push
1. **Step-by-step Granular Commits:**
   - When handling a task that includes multiple sub-features or fixes, commit each logical change separately as soon as it is implemented (e.g., implement feature A -> commit -> implement feature B -> commit).
   - Never combine unrelated features or fixes into a single monolithic commit.
2. **Clear & Descriptive Commit Messages:**
   - Use clear conventional commit titles (e.g., `feat(widgets): add custom slider`, `fix(menu): correct tab padding`).
3. **Automatic Stage & Push:**
   - Stage modified files (`git add .`), commit (`git commit -m "<descriptive message>"`), and push to remote (`git push`).
4. **Git Best Practices:**
   - Maintain a clean commit log allowing easy rollbacks (`git revert`, `git log`).
